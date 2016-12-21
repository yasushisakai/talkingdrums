
PImage img;
PImage new_img;
int[] cutpoints;

void setup(){
  size(1000,1000);
  img =loadImage("./data/in/myface.JPG");
  new_img = createImage(img.width,img.height,RGB);
  cutpoints = new int[height];

  int pitch = width/256;
  for(int i=0;i<height;i++){
    cutpoints[i] = int(random(256));
  }

  new_img.loadPixels();
  img.loadPixels();

  for(int i=0;i<img.pixels.length;i++){
    int x = i%img.width;
    int y = i/img.width;
    int new_x;


    if(y%2==0)
      new_x = x+img.width/2+cutpoints[y]*pitch;
    else
      new_x = x-img.width/2+cutpoints[y]*pitch;

    if(new_x < 0)
      new_x = img.width+abs(new_x);

      new_x = new_x%img.width;

    int new_id = y*img.width + new_x;
    try{
      new_img.pixels[new_id] = img.pixels[i];
    }catch(Exception e){
      println(new_x+", "+y);
    }
  }

  new_img.updatePixels();

  image(new_img,0,0);
  save("./data/out/myface_scrambled.jpg");
  exit();
}

void draw(){


}
