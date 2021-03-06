
// standard Arduino includes
#include <SPI.h>  // now we can use the SPI data transfer protocol          ( http://arduino.cc/en/Reference/SPI )
#include <math.h> // now we can do some more than the standard calculations ( http://www.arduino.cc/en/Math/H )
#include <MPU6000.h>
//#define DEBUG // comment this line for no DEBUG output

// ============================================================================================== //
//
//                          MAKE YOUR CHOICE FOR OUTPUTS HERE AND NOW !!!
//
// uncomment "#define OUTPUT_RAW_ACCEL" if you want to see the actual raw X, Y and Z accelerometer
// components from the MPU-6000 FIFO (as also available from registers 3B & 3C, 3D & 3E and 3F & 40)
// ! expected 1 g value around 16384, but from FIFO it is around 8192 (half of the expected value)
// - no big deal since raw values will not be used for stable roll, pitch and yaw angles
//#define OUTPUT_RAW_ACCEL
//
// uncomment "#define OUTPUT_RAW_ACCEL_G" if you want to see the same as with OUTPUT_RAW_ACCEL but
// recalculated to g-force values (just divived by 8192)
//#define OUTPUT_RAW_ACCEL_G
//
// uncomment "#define OUTPUT_RAW_ANGLES" if you want to see the calculated angles for roll and
// pitch from the raw acceleration components (yaw is undetermined then, this needs the use of the
// quaternion - see further on)
// around x-axis and y-axis: 0 to 360 degrees
// ! roll and pitch values calculated are somehow not independent from each other...hmmm...
//#define OUTPUT_RAW_ANGLES
//
// uncomment "#define OUTPUT_RAW_GYRO" if you want to see the actual raw X, Y and Z gyroscope
// components from the MPU-6000 FIFO (as also available from registers 43 & 44, 45 & 46 and 47 & 48)
//#define OUTPUT_RAW_GYRO
//
// uncomment "#define OUTPUT_TEMPERATURE" if you want to see the temperature of the MPU-6000 chip
// (which is not necessarily the same as the ambient temperature because of the power dissipation
// inside the MPU-6000 heating itself up a bit)
//#define OUTPUT_TEMPERATURE
//
// uncomment "#define OUTPUT_READABLE_QUATERNION" if you want to see the actual quaternion
// components from the MPU-6000 FIFO [w, x, y, z] (not best for parsing on a remote host such as
// Processing or something though)
//#define OUTPUT_READABLE_QUATERNION
//
// uncomment "OUTPUT_READABLE_EULER" if you want to see Euler angles (in degrees) calculated from
// the quaternions coming from the FIFO. Note that Euler angles suffer from gimbal lock (for more
// info, see http://en.wikipedia.org/wiki/Gimbal_lock )
// around x-axis and z-axis: -180 to +180 degrees, around y-axis 0 -> -90 -> 0 -> +90 -> 0
//#define OUTPUT_READABLE_EULER
//
// uncomment "OUTPUT_READABLE_ROLLPITCHYAW" if you want to see the yaw/pitch/roll angles (in
// degrees) calculated from the quaternions coming from the FIFO. Note this also requires gravity
// vector calculations. Also note that yaw/pitch/roll angles suffer from gimbal lock (for more
// info, see: http://en.wikipedia.org/wiki/Gimbal_lock )
// roll, pitch, yaw: 0, 0, 0 when in horizontal flight
// roll/pitch:  0 -> -90 -> 0 -> 90 -> 0
//        yaw: -180 to +180
//#define OUTPUT_READABLE_ROLLPITCHYAW
//
// uncomment "#define OUTPUT_TEAPOT" if you want output that matches the format used for the
// InvenSense Teapot demo - do not output anything else or the demo will not work
// - output looks like garbage, for instance $?h þ  but this is just what is expected 
#define OUTPUT_TEAPOT
//
//
// ============================================================================================== //


//Number of gyros
const int nGyros = 11;
//CS Pins
int gyros[nGyros]={2, 3, 4, 5, 6, 7, 8, 9, 10, A5, A4};// 2, 3, 4, 5, 6, 7, 8, 9, 10, A5, A4, A3, A2, A1, A0
MPU6000 MPU(gyros, nGyros);

// MPU control & status variables
boolean mpuIntStatus = false;
boolean dmpReady = false;     // set true if DMP initialization was successful
unsigned int packetSize = 42; // number of unique bytes of data written by the DMP each time (FIFO can hold multiples of 42-bytes)


// packet structure for InvenSense Teapot demo
byte teapotPacket[15] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, 0x00, '\r', '\n' };

// ############################################################################################## //
// ################################ SETUP ####################################################### //
// ############################################################################################## //
void setup()
{

  delay(5000);
  Serial.begin(115200);

  //Serial.println();
  //Serial.println("############# MPU-6000 Data Acquisition #############");

  //--- SPI settings ---//
  //Serial.println("Initializing MPUs...");
  MPU.gyrosInit();
 // Serial.println("...MPU Initialization done.");
  //Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  delay(100);


  // write & verify dmpMemory, dmpConfig and dmpUpdates into the DMP, and make all kinds of other settings
  // !!! this is the main routine to make the DMP work, and get the quaternion out of the FIFO !!!
  //Serial.println("Initializing Digital Motion Processor (DMP)...");
  boolean devStatus; // return status after each device operation (0 = success, !0 = error)
  devStatus = MPU.dmpInitialize();

    // make sure it worked: dmpInitialize() returns a true in devStatus if so
    if (devStatus == true)
    {
      for(int i = 0; i < nGyros; i++){ 
            // now that it's ready, turn on the DMP 
            //Serial.print("Enabling DMP... ");
            MPU.SPIwriteBit(0x6A, 7, true, gyros[i]); // USER_CTRL_DMP_EN
            //Serial.println("done.");

            byte mpuIntStatus = MPU.SPIread(0x3A, gyros[i]); // by reading INT_STATUS register, all interrupts are cleared
            //Serial.println("done.");

            // set our DMP Ready flag so the main loop() function knows it's okay to use it
            dmpReady = true;
            //Serial.println("DMP ready! Waiting for first data from MPU-6000...");
            //Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

            // show chosen outputs
           // Serial.println("You have chosen the following output(s):");
           /*
            #ifdef OUTPUT_RAW_ACCEL
            Serial.println("- OUTPUT_RAW_ACCEL");
            #endif
            #ifdef OUTPUT_RAW_ACCEL_G
            Serial.println("- OUTPUT_RAW_ACCEL_G");
            #endif
            #ifdef OUTPUT_RAW_ANGLES
            Serial.println("- OUTPUT_RAW_ANGLES");
            #endif
            #ifdef OUTPUT_RAW_GYRO
            Serial.println("- OUTPUT_RAW_GYRO");
            #endif
            #ifdef OUTPUT_TEMPERATURE
            Serial.println("- OUTPUT_TEMPERATURE");
            #endif
            #ifdef OUTPUT_READABLE_QUATERNION
            Serial.println("- OUTPUT_READABLE_QUATERNION");
            #endif
            #ifdef OUTPUT_READABLE_EULER
            Serial.println("- OUTPUT_READABLE_EULER");
            #endif
            #ifdef OUTPUT_READABLE_YAWPITCHROLL
            Serial.println("- OUTPUT_READABLE_YAWPITCHROLL");
            #endif
            #ifdef OUTPUT_TEAPOT
            //Serial.println("- OUTPUT_TEAPOT");
            #endif
            //Serial.println();
            */
          }

        }
   else // have to check if this is still functional
   {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    /*Serial.print("DMP Initialization failed (code ");
    Serial.print(devStatus);
    Serial.println(")");*/
    }

    
    //FIFOS reset
    if(dmpReady)
      for(int i = 0; i < nGyros; i++)
        MPU.SPIwriteBit(0x6A, 2, true, gyros[i]); // FIFO_RESET = 1 = reset (ok) only when FIFO_EN = 0

  } // end void setup()

// ############################################################################################## //
// ################################ MAIN LOOP ################################################### //
// ############################################################################################## //

void loop()
{

    // if DMP initialization during setup failed, don't try to do anything
    if (!dmpReady)
    return;

    if(Serial.available())
      if(Serial.read() == 'r')//restart
        asm volatile ("  jmp 0");  

    for(int i = 0; i < nGyros; i++){ //Mudar aqui numero de nGyros
      
      do{
        MPU.fifoCount = MPU.getFIFOCount(gyros[i]);
      }while (MPU.fifoCount < packetSize);
      


        byte mpuIntStatus = MPU.SPIread(0x3A, gyros[i]); // by reading INT_STATUS register, all interrupts are cleared

        // get current FIFO count
        MPU.fifoCount = MPU.getFIFOCount(gyros[i]);
        //Serial.print("fifoCount ");Serial.print(i);Serial.print(": ");Serial.println(MPU.fifoCount);

      // check for FIFO overflow (this should never happen unless our code is too inefficient or DEBUG output delays code too much)
      if ((mpuIntStatus & 0x10) || MPU.fifoCount >= 1008)
      {
        // reset so we can continue cleanly
        MPU.SPIwriteBit(0x6A, 2, true, gyros[i]); // FIFO_RESET = 1 = reset (ok) only when FIFO_EN = 0

        //Serial.print("FIFO overflow! FIFO resetted to continue cleanly. MPU : ");
        //Serial.println(i);
      }

      // otherwise, check for DMP data ready interrupt (this should happen frequently)
      else if (mpuIntStatus & 0x02)
        // mpuIntStatus & 0x02 checks register 0x3A for (undocumented) DMP_INT
        {    

        // read a packet from FIFO
        MPU.SPIreadBytes(0x74, packetSize, MPU.fifoBuffer, gyros[i]);

        // verify contents of fifoBuffer before use:
        # ifdef DEBUG
        for (byte n = 0 ; n < packetSize; n ++)
        {
          Serial.print("\tfifoBuffer["); Serial.print(n); Serial.print("]\t: "); Serial.println(fifoBuffer[n], HEX);
        }
        # endif

        // track FIFO count here in case there is more than one packet (each of 42 bytes) available
        // (this lets us immediately read more without waiting for an interrupt)
        MPU.fifoCount = MPU.fifoCount - packetSize;

    // ============================================================================================== //
    // >>>>>>>>> - from here the 42 FIFO bytes from the MPU-6000 can be used to generate output >>>>>>>>
    // >>>>>>>>> - this would be the place to add your own code into                            >>>>>>>>
    // >>>>>>>>> - of course all the normal MPU-6000 registers can be used here too             >>>>>>>>
    // ============================================================================================== //

        // get the quaternion values from the FIFO - needed for Euler and roll/pitch/yaw angle calculations
        /*
        int raw_q_w = ((MPU.fifoBuffer[0] << 8)  + MPU.fifoBuffer[1]);  // W
        int raw_q_x = ((MPU.fifoBuffer[4] << 8)  + MPU.fifoBuffer[5]);  // X
        int raw_q_y = ((MPU.fifoBuffer[8] << 8)  + MPU.fifoBuffer[9]);  // Y
        int raw_q_z = ((MPU.fifoBuffer[12] << 8) + MPU.fifoBuffer[13]); // Z
        float q_w = raw_q_w / 16384.0f;
        float q_x = raw_q_x / 16384.0f;
        float q_y = raw_q_y / 16384.0f;
        float q_z = raw_q_z / 16384.0f;*/

        #ifdef OUTPUT_RAW_ACCEL
          // print accelerometer values from fifoBuffer
          int AcceX = ((MPU.fifoBuffer[28] << 8) + MPU.fifoBuffer[29]);
          int AcceY = ((MPU.fifoBuffer[32] << 8) + MPU.fifoBuffer[33]);
          int AcceZ = ((MPU.fifoBuffer[36] << 8) + MPU.fifoBuffer[37]);
          Serial.print("Raw acceleration ax, ay, az [8192 = 1 g]: "); Serial.print("\t\t");
          Serial.print  (AcceX); Serial.print("\t");
          Serial.print  (AcceY); Serial.print("\t");
          Serial.println(AcceZ);
          #endif

          #ifdef OUTPUT_RAW_ACCEL_G
          // same as OUTPUT_RAW_ACCEL but recalculated to g-force values
          int AcceX = ((MPU.fifoBuffer[28] << 8) + MPU.fifoBuffer[29]);
          int AcceY = ((MPU.fifoBuffer[32] << 8) + MPU.fifoBuffer[33]);
          int AcceZ = ((MPU.fifoBuffer[36] << 8) + MPU.fifoBuffer[37]);
          float Ax = AcceX / 8192.0f; // calculate g-value
          float Ay = AcceY / 8192.0f; // calculate g-value
          float Az = AcceZ / 8192.0f; // calculate g-value
          Serial.print("Raw acceleration ax, ay, az [g]: "); Serial.print("\t\t\t");
          Serial.print  (Ax, 3); Serial.print("\t");
          Serial.print  (Ay, 3); Serial.print("\t");
          Serial.println(Az, 3);
          #endif

          #ifdef OUTPUT_RAW_ANGLES
          // print calculated angles for roll and pitch from the raw acceleration components
          // (yaw is undetermined here, this needs the use of the quaternion - see further on)
          int AcceX = ((MPU.fifoBuffer[28] << 8) + MPU.fifoBuffer[29]);
          int AcceY = ((MPU.fifoBuffer[32] << 8) + MPU.fifoBuffer[33]);
          int AcceZ = ((MPU.fifoBuffer[36] << 8) + MPU.fifoBuffer[37]);
          // atan2 outputs the value of -pi to pi (radians) - see http://en.wikipedia.org/wiki/Atan2
          // We then convert it to 0 to 2 pi and then from radians to degrees - in the end it's 0 - 360 degrees
          float ADegX = (atan2(AcceY, AcceZ) + PI) * RAD_TO_DEG;
          float ADegY = (atan2(AcceX, AcceZ) + PI) * RAD_TO_DEG;
          Serial.print("Calculated angle from raw acceleration - roll, pitch and yaw [degrees]: ");
          Serial.print(ADegX); Serial.print("\t");
          Serial.print(ADegY); Serial.print("\t");
          Serial.println("undetermined");
          #endif

          #ifdef OUTPUT_RAW_GYRO
          // print gyroscope values from fifoBuffer
          int GyroX = ((MPU.fifoBuffer[16] << 8) + MPU.fifoBuffer[17]);
          int GyroY = ((MPU.fifoBuffer[20] << 8) + MPU.fifoBuffer[21]);
          int GyroZ = ((MPU.fifoBuffer[24] << 8) + MPU.fifoBuffer[25]);
          Serial.print("Raw gyro rotation ax, ay, az [value/deg/s]: "); Serial.print("\t\t");
          Serial.print(GyroX); Serial.print("\t");
          Serial.print(GyroY); Serial.print("\t");
          Serial.println(GyroZ);
          #endif

          #ifdef OUTPUT_TEMPERATURE
          // print calculated temperature from standard registers (not available in fifoBuffer)
          // the chip temperature may be used for correction of the temperature sensitivity of the
          // accelerometer and the gyroscope - not done in this sketch 
          byte Temp_Out_H = SPIread(0x41,ChipSelPin);
          byte Temp_Out_L = SPIread(0x42,ChipSelPin);
          int TemperatureRaw = Temp_Out_H << 8 | Temp_Out_L;
          float Temperature = (TemperatureRaw / 340.00) + 36.53; // formula from datasheet chapter 4.19
          Serial.print("Chip temperature for corrections [deg. Celsius]: ");
          Serial.println(Temperature, 2);
          #endif

          #ifdef OUTPUT_READABLE_QUATERNION
          // Serial.print("Quaternion qw, qx, qy, qz [-1 to +1]: "); Serial.print("\t");
          // Serial.print  (q_w); Serial.print("\t");
          // Serial.print  (q_x); Serial.print("\t");
          // Serial.print  (q_y); Serial.print("\t");
          // Serial.println(q_z);
          Serial.print("$"); Serial.print(" "); Serial.print(i); Serial.println(" ");
          //Serial.print(q_x*100); Serial.print(" ");
          //Serial.print(q_y*100); Serial.print(" ");
          //Serial.print(q_z*100); Serial.print(" ");
          //Serial.println(q_w*100);
          #endif

          #ifdef OUTPUT_READABLE_EULER
        // calculate Euler angles
        // http://en.wikipedia.org/wiki/Atan2
        // http://en.wikipedia.org/wiki/Sine (-> Inverse) 
        float euler_x = atan2((2 * q_y * q_z) - (2 * q_w * q_x), (2 * q_w * q_w) + (2 * q_z * q_z) - 1); // phi
        float euler_y = -asin((2 * q_x * q_z) + (2 * q_w * q_y));                                        // theta
        float euler_z = atan2((2 * q_x * q_y) - (2 * q_w * q_z), (2 * q_w * q_w) + (2 * q_x * q_x) - 1); // psi
        euler_x = euler_x * 180/M_PI; // angle in degrees -180 to +180
        euler_y = euler_y * 180/M_PI; // angle in degrees
        euler_z = euler_z * 180/M_PI; // angle in degrees -180 to +180
        Serial.print("Euler angles x, y, z [degrees]: ");
        Serial.print(euler_x); Serial.print("\t");
        Serial.print(euler_y); Serial.print("\t");
        Serial.print(euler_z); Serial.println();
        #endif

        #ifdef OUTPUT_READABLE_ROLLPITCHYAW
          // display Euler angles in degrees

          // dmpGetGravity
          float grav_x = 2 * ((q_x * q_z) - (q_w * q_y));
          float grav_y = 2 * ((q_w * q_x) + (q_y * q_z));
          float grav_z = (q_w * q_w) - (q_x * q_x) - (q_y * q_y) + (q_z * q_z);

          // roll: (tilt left/right, about X axis)
          float rpy_rol = atan(grav_y / (sqrt((grav_x * grav_x) + (grav_z * grav_z))));
          // pitch: (nose up/down, about Y axis)
          float rpy_pit = atan(grav_x / (sqrt((grav_y * grav_y) + (grav_z * grav_z))));
          // yaw: (rotate around Z axis)
          float rpy_yaw = atan2((2 * q_x * q_y) - (2 * q_w * q_z), (2 * q_w * q_w) + (2 * q_x * q_x) - 1);

          Serial.print("Roll, pitch and yaw angles [degrees]: ");
          Serial.print(rpy_rol * 180/M_PI); Serial.print("\t");
          Serial.print(rpy_pit * 180/M_PI); Serial.print("\t");
          Serial.print(rpy_yaw * 180/M_PI); Serial.println();
          #endif

          #ifdef OUTPUT_TEAPOT
          // display quaternion values in InvenSense Teapot demo format:
          teapotPacket[2] = MPU.fifoBuffer[0];
          teapotPacket[3] = MPU.fifoBuffer[1];
          teapotPacket[4] = MPU.fifoBuffer[4];
          teapotPacket[5] = MPU.fifoBuffer[5];
          teapotPacket[6] = MPU.fifoBuffer[8];
          teapotPacket[7] = MPU.fifoBuffer[9];
          teapotPacket[8] = MPU.fifoBuffer[12];
          teapotPacket[9] = MPU.fifoBuffer[13];
          teapotPacket[12] = i;
          Serial.write(teapotPacket, 15);
          teapotPacket[11]++; // packetCount, loops at 0xFF on purpose
          #endif

    // ============================================================================================== //
    // >>>>>>>>> - this would normally be the end of adding your own code into this sketch          >>>>
    // >>>>>>>>> - end of using the 42 FIFO bytes from the MPU-6000 to generate output              >>>>
    // >>>>>>>>> - after blinking the red LED, the main loop starts again (and again, and again...) >>>>
    // ============================================================================================== //
  }
}


  } // end void loop()
