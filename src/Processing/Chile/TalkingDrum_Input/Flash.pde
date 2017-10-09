PGraphics flash;
boolean activateFlash = false;
int timeCounter = 0;
int maxCounter = 4;

void flash(){
  flash = createGraphics(width, height);
  
  flash.beginDraw();
  flash.background(255);
  flash.endDraw();
}