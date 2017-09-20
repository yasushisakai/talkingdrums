class Device {
  
  int id = -1;
  boolean isChecked = false;
  int x = 0;
  int y = 0;
  int size = 10;
  
  Device(int _id,int _x,int _y){
    id = _id;
    x = _x;
    y = _y;
  }
  
  void draw() {
    if(isChecked) {
      fill(255, 0, 0);
    } else {
      fill(255);
    }
    
    pushMatrix();
    translate(x, y);
    rect(0, 0, size, size);
    popMatrix();
 
  }
  
  void check(int mX, int mY) {
    if ((x > mX && x < mX + size) && (y > mY && y < mY + size)) {
      isChecked = !isChecked;
    }
  }
}