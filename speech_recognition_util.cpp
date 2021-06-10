//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
//

// <toplevel>
#include <iostream>
#include <speechapi_cxx.h>
#include <fstream>
#include <unistd.h>
#include "wav_file_reader.h"

using namespace std;
using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;
// </toplevel>

void SpeechRecognitionWithPullStream(const int &sock_fd)
{
    // First, define your own pull audio input stream callback class that implements the
    // PullAudioInputStreamCallback interface. The sample here illustrates how to define such
    // a callback that reads audio data from a wav file.
    // AudioInputFromFileCallback implements PullAudioInputStreamCallback interface, and uses a wav file as source
    class AudioInputFromFileCallback final : public PullAudioInputStreamCallback
    {
    public:
        // Constructor that creates an input stream from a file.
        AudioInputFromFileCallback(const int& sock):sock_fd(sock), read_count(0) {}

        // Implements AudioInputStream::Read() which is called to get data from the audio stream.
        // It copies data available in the stream to 'dataBuffer', but no more than 'size' bytes.
        // If the data available is less than 'size' bytes, it is allowed to just return the amount of data that is currently available.
        // If there is no data, this function must wait until data is available.
        // It returns the number of bytes that have been copied in 'dataBuffer'.
        // It returns 0 to indicate that the stream reaches end or is closed.
        int Read(uint8_t* dataBuffer, uint32_t size) override
        {
            read_count++;
						cout<<"Calling the Read method - "<<read_count<<", size - "<<size<<endl;
            return read(sock_fd, dataBuffer, size);
        }
        // Implements AudioInputStream::Close() which is called when the stream needs to be closed.
        void Close() override
        {
						cout<<"Calling the Close Method"<<endl;
            close(sock_fd);
        }

    private:
        int sock_fd;
        int read_count;
    };

    // Creates an instance of a speech config with specified subscription key and service region.
    // Replace with your own subscription key and service region (e.g., "westus").
    auto config = SpeechConfig::FromSubscription("<Subscription_Key", "<Region>");

    // Creates a callback that will read audio data from a WAV file.
    // Currently, the only supported WAV format is mono(single channel), 16 kHZ sample rate, 16 bits per sample.
    // Replace with your own audio file name.
    auto callback = make_shared<AudioInputFromFileCallback>(sock_fd);
    auto pullStream = AudioInputStream::CreatePullStream(callback);

    // Creates a speech recognizer from stream input;
    auto audioInput = AudioConfig::FromStreamInput(pullStream);
    auto recognizer = SpeechRecognizer::FromConfig(config, audioInput);

    // promise for synchronization of recognition end.
    promise<void> recognitionEnd;

	recognizer->SessionStarted.Connect([&recognitionEnd](const SessionEventArgs& e)
    {
        cout << "Session Started."<<endl;
    });

    // Subscribes to events.
    recognizer->Recognizing.Connect([](const SpeechRecognitionEventArgs& e)
    {
        cout<<"Recognizing:"<<e.Result->Text<<std::endl;
    });

    // recognizer->Recognized.Connect([] (const SpeechRecognitionEventArgs& e)
    // {
    //     if (e.Result->Reason == ResultReason::RecognizedSpeech)
    //     {
    //         cout << "RECOGNIZED : Text = " << e.Result->Text << std::endl
    //              << "  Offset=" << e.Result->Offset() << std::endl
    //              << "  Duration=" << e.Result->Duration() << std::endl;
    //     }
    //     else if (e.Result->Reason == ResultReason::NoMatch)
    //     {
    //         cout << "NOMATCH: Speech could not be recognized." << std::endl;
    //     }
    // });

    recognizer->Canceled.Connect([&recognitionEnd](const SpeechRecognitionCanceledEventArgs& e)
    {
        switch (e.Reason)
        {
        case CancellationReason::EndOfStream:
            cout << "CANCELED: Reach the end of the file." << std::endl;
            break;

        case CancellationReason::Error:
            cout << "CANCELED: ErrorCode=" << (int)e.ErrorCode << std::endl;
            cout << "CANCELED: ErrorDetails=" << e.ErrorDetails << std::endl;
            recognitionEnd.set_value();
            break;

        default:
            cout << "unknown reason ?!" << std::endl;
        }
    });

    recognizer->SessionStopped.Connect([&recognitionEnd](const SessionEventArgs& e)
    {
        cout << "Session stopped."<<endl;
        recognitionEnd.set_value(); // Notify to stop recognition.
    });


    auto result = recognizer->RecognizeOnceAsync().get();

    // Checks result.
    if (result->Reason == ResultReason::RecognizedSpeech)
    {
        cout << "RECOGNIZED Output from RecognizeOnceAsync: Text=" << result->Text << std::endl;
    }
    else if (result->Reason == ResultReason::NoMatch)
    {
        cout << "NOMATCH: Speech could not be recognized." << std::endl;
    }
    else if (result->Reason == ResultReason::Canceled)
    {
        auto cancellation = CancellationDetails::FromResult(result);
        cout << "CANCELED: Reason=" << (int)cancellation->Reason << std::endl;

        if (cancellation->Reason == CancellationReason::Error)
        {
            cout << "CANCELED: ErrorCode=" << (int)cancellation->ErrorCode << std::endl;
            cout << "CANCELED: ErrorDetails=" << cancellation->ErrorDetails << std::endl;
            cout << "CANCELED: Did you update the subscription info?" << std::endl;
        }
    }



    // // Starts continuous recognition. Uses StopContinuousRecognitionAsync() to stop recognition.
    // recognizer->StartContinuousRecognitionAsync().wait();

    // // Waits for recognition end.
    // recognitionEnd.get_future().wait();

    // // Stops recognition.
    // recognizer->StopContinuousRecognitionAsync().wait();
}

