tempo 120
compasses 16
steps 4

pattern kickPattern {
    rhythm [x,.,x,.] * 4
}

pattern snarePattern {
    rhythm [.,x,.,x] * 4
}

pattern hihatPattern {
    rhythm [x,x,x,x] * 4
}

pattern bassPattern {
    rhythm [E2,G2,A2,.] + [E2,G2,A#2,A2] + [E2,G2,A2,.] + [G2,E2,-,-]
}

instruments {
    kick {
        pattern kickPattern
        active 1-16
    }
    snare {
        pattern snarePattern
        active 5-16
    }
    hihat {
        pattern hihatPattern
        active 9-16
    }
    bass {
        pattern bassPattern
        active 1-16
    }
}
