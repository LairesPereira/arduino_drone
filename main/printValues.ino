
void printValues() {
  
 Serial.print(" ");
 Serial.print(2500);
 Serial.print(" ");
 Serial.print(roll_PID);
 Serial.print(" ");
 Serial.print(pitch_PID);
 Serial.print(" RF ");
 Serial.print(pwm_R_F);
 //Serial.print("   |   ");
 Serial.print(" RB ");
 Serial.print(pwm_R_B);
 //Serial.print("   |   ");
 Serial.print(" ");
 Serial.print(pwm_L_B);
 //Serial.print("   |   ");
 Serial.print(" ");
 Serial.println(pwm_L_F);

/*
 Serial.print("   |   ");
 Serial.print("Xº: ");
 Serial.print(Total_angle_x);
 Serial.print("   |   ");
 Serial.print("Yº: ");
 Serial.print(Total_angle_y);
 Serial.println(" ");
*/

}