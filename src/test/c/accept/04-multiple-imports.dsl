remember "lib/kick.dsl"
remember "lib/snare.dsl"
remember "lib/hihat.dsl"

tempo 140
compasses 16
steps 8

pattern custom {
    rhythm [x,x,.,x] * 2
}

instruments {
    custom_inst {
        pattern custom
        active 1-16
    }
}
