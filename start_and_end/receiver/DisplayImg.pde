class DisplayImg {
  PImage img;
  String imgUrl = "https://cityio.media.mit.edu/talkingdrums/image/current/";
  int pTime;
  int maxTime;


  DisplayImg() {
    img = loadImage(imgUrl, "png");
    maxTime = 3000;
  }

  void update() {
    int cTime = millis();
    if ((cTime - pTime) > maxTime) {
      img = loadImage(imgUrl, "png");
      
      //rest time
      pTime = cTime;
    }
  }

  void drawFullScreen() {
    image(img, 0, 0, width, height);
  }
}
