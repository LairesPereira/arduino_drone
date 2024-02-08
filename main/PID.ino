double kp = 5;
double kd = 0;
double ki = 0;

int OUTMAX = 2000;
int OUTMIN = 1000;

double setPoint = 0;

float lastInput = 0;

double ITerm = 0.0;

double Compute(double input) {
  double erro = setPoint - input; // calculo erro
  ITerm += (ki * erro);
 // Serial.print(" | FstITerm:  ");
 //Serial.print(ITerm);

  if(ITerm > OUTMAX) { 
    ITerm = OUTMAX;
  } else  {
    ITerm = OUTMIN;
  }

  double dInput = input - lastInput;

  double output = kp * erro + ITerm + kd * dInput;
  
  if(output > OUTMAX) {
    output = OUTMAX;
  } else if (output <= OUTMIN) {
    output = OUTMIN; 
  }

  //Serial.print(" | Erro: ");
 // Serial.print(erro);
  //Serial.print(" | ITerm: ");
  //Serial.print(ITerm);

  lastInput = input;
  return output;
}
