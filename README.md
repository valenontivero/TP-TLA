# TP-TLA

### Ejemplo a priori del lenguaje  
```
tempo{124};
compases{4};
steps{1};
instruments{
  kick{
    rithm{4*[.-.-]};
    duration{1};
  }
  bass{
    rithm{ [E2,G2,A2, -] + [E2,G2,A#2,A2] + [E2,G2,A2, -] + [-, G2, E2, -] };
    duration{1};
  }
}
```

tempo: la velocidad general a la que sucederá el ritmo.
compases: la cantidad de compases que se loopearán en total.
steps: cantidad de golpes utilizables por cada pulso (beat).
intstrumentos: aquí se listan los instrumentos que aparecerán
