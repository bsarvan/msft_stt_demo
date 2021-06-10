#
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
#

SPEECHSDK_ROOT:=/Users/bharat.sarvan/speechsdk

CHECK_FOR_SPEECHSDK := $(shell test -f $(SPEECHSDK_ROOT)/MicrosoftCognitiveServicesSpeech.framework/MicrosoftCognitiveServicesSpeech && echo Success)
ifneq ("$(CHECK_FOR_SPEECHSDK)","Success")
  $(error Please set SPEECHSDK_ROOT to point to your extracted Speech SDK, $$SPEECHSDK_ROOT/MicrosoftCognitiveServicesSpeech.framework/MicrosoftCognitiveServicesSpeech should exist.)
endif


# If you'd like to build for 32-bit Linux, replace "x64" in the next line with "x86".
INCPATH:=$(SPEECHSDK_ROOT)/MicrosoftCognitiveServicesSpeech.framework/Headers

LIBS:=-framework MicrosoftCognitiveServicesSpeech

all: stt_service stt_client

# Note: to run, LD_LIBRARY_PATH should point to $LIBPATH.
stt_service: speech_recognition.cpp speech_recognition_util.cpp
	g++ $^ -o $@ \
	    --std=c++14 \
	    $(patsubst %,-I%, $(INCPATH)) \
	    $(patsubst %,-F%, $(SPEECHSDK_ROOT)) \
	    $(LIBS)

stt_client: speech_recognition_client.cpp
	g++ $^ -o $@ \
	--std=c++14

clean:
	rm stt_service stt_client
