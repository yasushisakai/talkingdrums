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
      try{
      img = loadImage(imgUrl, "png");
      } catch (Exception e) {
        println("Erro: get request in image");
      }
      
      //rest time
      pTime = cTime;
    }
  }

  void drawFullScreen() {
    if (img.width > 0 || img.height > 0 ) {
      image(img, 0, 0, width, height);
    }
  }
}
