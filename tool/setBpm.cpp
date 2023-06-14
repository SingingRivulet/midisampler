#include "Options.h"
#include "midiLoader.h"
#include "midiMap.h"
#include "sampler.h"
int main(int argc, char** argv) {
    if (argc < 3) {
        return 0;
    }
    std::string input = argv[1];
    std::string output = argv[2];
    smf::MidiFile midifile1;
    midifile1.read(input);
    smf::MidiFile midifile2;
    midifile2.read(output);
    midifile2.setTPQ(midifile1.getTicksPerQuarterNote());
    midifile2.write(output);
    return 0;
}