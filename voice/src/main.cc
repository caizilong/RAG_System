// main.cc

#include <signal.h>

#include "conversation_manager.h"
#include "recorder.h"
#include "sherpa-onnx/csrc/microphone.h"

static bool global_stop = false;

static void Handler(int sig) { global_stop = true; }

int main(int argc, char* argv[]) {
    signal(SIGINT, Handler);

    // PortAudio init/terminate by Microphone class
    sherpa_onnx::Microphone mic;

    ConversationManager conv(argc, argv);
    Recorder recorder(conv);

    recorder.Run();
    return 0;
}
