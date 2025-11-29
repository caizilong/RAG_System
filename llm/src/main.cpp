#include "VoiceLLMService.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage: %s model_path\n", argv[0]);
        return 1;
    }

    VoiceLLMService service(argv[1]);
    service.runForever();

    return 0;
}
