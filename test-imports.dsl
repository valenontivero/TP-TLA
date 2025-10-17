remember "patterns/kick.dsl"
remember "patterns/snare.dsl"

tempo 120
compasses 8
steps 4

pattern hihat {
    rhythm [x,x,x,x]
}

instruments {
    hh {
        pattern hihat
        active 1-8
    }
}
