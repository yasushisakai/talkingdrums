class DisplayImg{
  PImage img;
  String imgUrl = "https://cityio.media.mit.edu/talkingdrums/image/original/";
  
  DisplayImg(){
    img = loadImage(imgUrl);
  }
  
  void drawFullScreen(){
    image(img, 0, 0, width, height); 
  }
  
}
