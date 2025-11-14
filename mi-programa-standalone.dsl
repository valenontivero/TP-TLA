tempo 120
compasses 8
steps 4

pattern kickPattern {
    rhythm [x,.,x,.]
}

pattern bassPattern {
    rhythm [E2,.,A2,.]
}

instruments {
    kick {
        pattern kickPattern
        active 1-8
    }
    bass {
        pattern bassPattern
        active 1-8
    }
}
