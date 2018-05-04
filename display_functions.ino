void configDisplay(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
  display.setTextSize(1);
}

void printOnDisplay(int line, String text){
  display.setCursor(0,line*10);
  display.print(text);
  display.display();
}

void appendOnDisplay(String text){
  display.print(text);
  display.display();
}

void clearDisplay(){
  display.clearDisplay();  
  display.setCursor(0,0);
  display.display();
}

