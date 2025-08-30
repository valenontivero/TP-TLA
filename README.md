# TP-TLA

### Ejemplo a priori del lenguaje  
```
tempo{124};
compases{4};
steps{1};
instruments{
  kick{
    rithm{4 * [x,-,x,-]};
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
instruments: aquí se listan los instrumentos que aparecerán dentro del ritmo.  
rythm: es el patrón de notas (o golpes) que seguirá el instrumento. En este caso el kick es 4 veces golpes en el primer y tercer pulso de cada compás.  
duration: es la cantidad de "steps" que se mantiene cada símbolo del "rithm".  
