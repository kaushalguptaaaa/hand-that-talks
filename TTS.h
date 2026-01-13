#ifndef TTS_H
#define TTS_H

#include <string>
#include <cstdlib>
#include <iostream>

class TTS {
public:
    static void speak(const std::string& text) {
        if (text.empty()) return;
        
        // Escape quotes if necessary, simpler command for now
        std::string command = "powershell -Command \"Add-Type -AssemblyName System.Speech; (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak('" + text + "');\"";
        
        // Execute in background? system() blocks. 
        // For < 1s latency, blocking might be bad if speech is long.
        // But SpeechSynthesizer.SpeakAsync is better.
        
        command = "powershell -Command \"Add-Type -AssemblyName System.Speech; (New-Object System.Speech.Synthesis.SpeechSynthesizer).SpeakAsync('" + text + "');\"";
        
        system(command.c_str());
    }
};

#endif
