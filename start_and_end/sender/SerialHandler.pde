import processing.serial.*;

final byte RETURN = 0b00001010;
final byte READY = 0b00000000;
final byte SEND = 0b00000001;
final byte RECEIVE = 0b00000010;
final int BAUD = 115200;
final boolean DEBUG = true;

class SerialHandler{

  public Serial port;

  SerialHandler(PApplet p) throws Exception {
    this.connect(p);
  }

  public void connect(PApplet p) throws Exception{
    String[] portList = Serial.list();
    String portName = portList[portList.length - 1];
      try{ 
        this.port = new Serial(p, portName, BAUD);
      } catch (Exception e) {
        println("Error connecting to Serial :" + e);
        throw e;
      }

    println("connected to: " + portName);
  }

  public boolean checkReady() throws Exception {
    while(this.port.available() > 0){
      byte[] incomming = new byte[]{0, 0, 0}; 
      incomming = this.port.readBytes(3);
      // printArray(incomming);
      try{
      return incomming[0] == READY && incomming[2] == RETURN;
      } catch (Exception e) {
        return false;
      }
    }
    return false;
  }
  
  public byte getByte() throws Exception{
    while(this.port.available() > 0){
      byte[] incomming = new byte[]{0, 0, 0};
      incomming = this.port.readBytes(3);
      if(DEBUG){
        printArray(incomming);
      }
      
      if(incomming.length < 3) {
        throw new Exception("index out of bounds: message length" + incomming.length);
      }
      
      if(incomming[0] == RECEIVE && incomming[2] ==RETURN){
        return (byte)incomming[1];
      } else{
        throw new Exception("message wrong protocol");
      }
    }
    throw new Exception("invalid message");
  }

  public void sendByte(byte data){
    // compose message 
    byte[] m = new byte[3];
    m[0] = SEND;
    m[1] = data;
    m[2] = RETURN; 

    this.port.write(m);
    this.port.clear();
  }
}
