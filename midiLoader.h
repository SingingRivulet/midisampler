#ifndef MGNR_LOADER
#define MGNR_LOADER
#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
namespace mgnr{

template<typename T>
inline void loadMidi(T & self,const std::string & str){
    const char * instrumentName[] = {
        "Piano",
        "BrightPiano",
        "ElectricPiano",
        "HonkyTonkPiano",
        "RhodesPiano",
        "ChorusedPiano",
        "Harpsichord",
        "Clavinet",
        "Celesta",
        "Glockenspiel",
        "MusicBoX",
        "Vibraphone",
        "Marimba",
        "Xylophone",
        "TubularBells",
        "Dulcimer",
        "HammondOrgan",
        "PercussiveOrgan",
        "RockOrgan",
        "ChurchOrgan",
        "ReedOrgan",
        "Accordion",
        "Harmonica",
        "TangoAccordian",
        "Guitar-nylon",
        "Guitar-steel",
        "Guitar-jazz",
        "Guitar-clean",
        "Guitar-muted",
        "OverdrivenGuitar",
        "DistortionGuitar",
        "GuitarHarmonics",
        "AcousticBass",
        "ElectricBass-finger",
        "ElectricBass-pick",
        "FretlessBass",
        "SlapBass1",
        "SlapBass2",
        "SynthBass1",
        "SynthBass2",
        "Violin",
        "Viola",
        "Cello",
        "Contrabass",
        "TremoloStrings",
        "PizzicatoStrings",
        "OrchestralHarp",
        "Timpani",
        "StringEnsemble1",
        "StringEnsemble2",
        "SynthStrings1",
        "SynthStrings2",
        "ChoirAahs",
        "VoiceOohs",
        "SynthVoice",
        "OrchestraHit",
        "Trumpet",
        "Trombone",
        "Tuba",
        "MutedTrumpet",
        "FrenchHorn",
        "BrassSection",
        "SynthBrass1",
        "SynthBrass2",
        "SopranoSaX",
        "AltoSaX",
        "TenorSaX",
        "BaritoneSaX",
        "Oboe",
        "EnglishHorn",
        "Bassoon",
        "Clarinet",
        "Piccolo",
        "Flute",
        "Record",
        "PanFlute",
        "BottleBlow",
        "Skakuhachi",
        "Whistle",
        "Ocarina",
        "Lead1-square",
        "Lead2-sawtooth",
        "Lead3-calliope",
        "Lead4-chiff",
        "Lead5-charang",
        "Lead6-voice",
        "Lead7-fifths",
        "Lead8-bass",
        "Pad1-newage",
        "Pad2-warm",
        "Pad3-polysynth",
        "Pad4-choir",
        "Pad5-bowed",
        "Pad6-metallic",
        "Pad7-halo",
        "Pad8-sweep",
        "FX1-rain",
        "FX2-soundtrack",
        "FX3-crystal",
        "FX4-atmosphere",
        "FX5-brightness",
        "FX6-goblins",
        "FX7-echoes",
        "FX8-sci-fi",
        "Sitar",
        "Banjo",
        "Shamisen",
        "Koto",
        "Kalimba",
        "Bagpipe",
        "Fiddle",
        "Shanai",
        "Tinkle Bell",
        "Agogo",
        "SteelDrums",
        "Woodblock",
        "TaikoDrum",
        "MelodicTom",
        "SynthDrum",
        "ReverseCymbal",
        "GuitarFretNoise",
        "BreathNoise",
        "Seashore",
        "BirdTweet",
        "TelephoneRing",
        "Helicopter",
        "Applause",
        "Gunshot"
    };
    smf::MidiFile midifile;
    midifile.read(str);
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();
    
    int TPQ=midifile.getTicksPerQuarterNote();
    self.TPQ = TPQ;
    std::cout << "TPQ: " << TPQ << std::endl;
    int tracks = midifile.getTrackCount();
    
    if (tracks > 1)
        std::cout << "TRACKS: " << tracks << std::endl;
    
    std::set<int> iset;
    
    for (int track=0; track<tracks; track++) {
        //if (tracks > 1) cout << "\nTrack " << track << endl;
        
        char infoBuf[128];
        
        int instrumentId = 0;
        
        for (int event=0; event<midifile[track].size(); event++) {
            
            if (midifile[track][event].isNoteOn() && midifile[track][event].size()>=3){
                int position = midifile[track][event].tick;
                int delay = midifile[track][event].getTickDuration();
                int delayS = midifile[track][event].getDurationInSeconds();
                int tone = (int)midifile[track][event][1];
                int v = (int)midifile[track][event][2];
                snprintf(infoBuf,sizeof(infoBuf),"%s.%d",instrumentName[instrumentId],track);
                self.addNote(position, tone, delay, v,infoBuf);
                iset.insert(instrumentId);
            }else if(midifile[track][event].isTimbre()){
                instrumentId = midifile[track][event].getP1();
                if(instrumentId<0)
                    instrumentId = 0;
                else if(instrumentId>128)
                    instrumentId = 128;
            }
        }
    }
    
    for(int i=0; i<midifile.getNumEvents(0); i++){
        if(midifile.getEvent(0,i).isTempo()){//是设置时间
            double tp = midifile.getEvent(0,i).getTempoBPM();
            self.addTempo(midifile.getEvent(0,i).tick,tp);
        }
    }
    
    //for (auto it : iset){
    //    loadInstrument(it);
    //}
}

}
#endif
