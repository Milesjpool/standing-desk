
// #define MAX_MILLIS_TO_WAIT 1000  //or whatever
// #define DOWNPIN  12
// #define UPPIN    27
// #define UPOP  1
// #define DNOP  2
// #define NOOP  0
// #define ACTIVATE LOW 
// #define DEACTIVATE HIGH
// #define DESKMOVEDDURINGREAD 9

// int currentOperation = NOOP;
// unsigned long starttime;
// int requestedHeight = 0;
// int heightMultiplier = 1; 


// /* This method stops moving and sets the requested height to current height */
// void stopMoving(void) {
  // digitalWrite(DOWNPIN,DEACTIVATE);
  // digitalWrite(UPPIN,DEACTIVATE);
  // currentHeight = requestedHeight;
  // currentOperation = NOOP;
// }

// void setHeight(int height) {

//   int operation = 0;
//   requestedHeight = height;
//   if(currentHeight > requestedHeight) {
//     operation = DNOP;
//   };
//   if(currentHeight < requestedHeight) {
//     operation = UPOP;
//   };

//   if(currentHeight == requestedHeight) {
//     operation = NOOP;
//   };
//   switch(operation){
//     case UPOP:
//       digitalWrite(UPPIN,ACTIVATE);
//       currentOperation = UPOP;
//       Serial.println("going up");
//     break;
//     case DNOP:
//       digitalWrite(DOWNPIN,ACTIVATE);
//       currentOperation = DNOP;
//       Serial.println("going down");
//     break;

//     case NOOP:
//       currentOperation = NOOP;
//       stopMoving();
//       break;
    
//     default:
//       stopMoving();
//     break; 
//   };
// }
