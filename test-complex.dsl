tempo 80
compasses 4
steps 4

pattern kickPattern {
    rhythm [x,.,x,.]
}

pattern bassPattern {
    rhythm [E4,G4,A4,.] + [E4,G4,B4,Bb4]
}

pattern hihatPattern {
    rhythm [.,x,.,.]
}

instruments {
    kick {
        pattern kickPattern
        active 1-4
    }
    bass {
        pattern bassPattern
        active 2-4
    }
    hihat {
        pattern hihatPattern
        active 1-2
    }
}
