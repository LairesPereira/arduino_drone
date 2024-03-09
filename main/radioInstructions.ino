void radioInstructions() {
  if(radio.available()) {
    Serial.print(" Radio ");
      char controllerInstruction[32] = "";
      char directionInstruction[32] = "";
      radio.read(&controllerInstruction, sizeof(controllerInstruction));
      radio.read(&directionInstruction, sizeof(directionInstruction));
      Serial.println(controllerInstruction);
      Serial.println(directionInstruction);    
      
      if(strcmp(controllerInstruction, "EMERGENCY BREAK") == 0) {
          exit(1);
      } else if(strcmp(controllerInstruction, "GENERAL_SPEED") == 0) {
        
        if(strcmp(directionInstruction, "UP") == 0) {
          input_THROTTLE += 10;
          
        } else if(strcmp(directionInstruction, "DOWN") == 0) {
          input_THROTTLE -= 10;
        }

      } else if(strcmp(controllerInstruction, "DIRECTION") == 0) {
            if(strcmp(directionInstruction, "RIGHT") == 0 && pitch_desired_angle > -20) {
              pitch_desired_angle -= 1;
            } else if(strcmp(directionInstruction, "LEFT") == 0 && pitch_desired_angle < 20) {
              pitch_desired_angle += 1;
            }
        }
    }
}