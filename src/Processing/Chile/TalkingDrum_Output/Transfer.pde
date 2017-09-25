
///save all the incomming pixels locally

//if closed the app, save the image generated.

void saveImage() {

  int s = second();  // Values from 0 - 59
  int mi = minute(); 
  int d = day();    // Values from 1 - 31
  int m = month();  // Values from 1 - 12
  int y = year();

  String timeString = "td_"+y+"_"+m+"_"+d+"_"+mi+"_"+s
  
  println("saved "+timeString);

  pg.save("../output_images/"+timeString+".png");
}