tempo 115
compasses 4
steps 4

pattern kickPattern {
    rhythm [x,.,x,.] * 4
}

pattern bassPattern {
    rhythm [E2,G2,A2,.] + [E2,G2,A#2,A2] + [E2,G2,A2,.] + [G2, E2,.,.]
}

instruments {
    kick {
        pattern kickPattern
        active 1-4
    }
    bass {
        pattern bassPattern
        active 1-4
    }
}