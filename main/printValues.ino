
void printValues() {
  Serial.print(" AngleX: ");
  Serial.print(Total_angle[1]);
  Serial.print(" Desired Angle: ");
  Serial.print(desired_angle);
  Serial.print(" PID: ");
  Serial.print(PID);
  Serial.print(" Throttle: ");
  Serial.print(throttle);
  Serial.print(" Motor Left: ");
  Serial.print(pwmLeft);
  Serial.print(" Motor Right: ");
  Serial.println(pwmRight);
}