const int tagLen = 10;
const int idLen = 9;
const int kTags = 4;

int tagIndx = -1;

char knownTags[kTags][idLen] ;
char tempTags[kTags][idLen];
char addTag[idLen] = "07454582";
char delTag[idLen] = "07502485";
char newTag[idLen];

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop()
{
  
  // put your main code here, to run repeatedly:
  readTag();
  if(strlen(newTag)==0)
  return;
  else
  access();
  
}

int checkTag(char nTag[], char oTag[]){
  Serial.print("Checking for: ");
  Serial.print(oTag);
  for(int i=0;i<idLen;i++){
    if(nTag[i]!=oTag[i]){
      Serial.println(" False");
      return 0;
    }
  }
  Serial.println(" True");
  return 1;
}

int readTag(){
  for (int c=0 ; c<idLen ; c++){
    newTag[c] = 0;
  }
  int i = 0;
  int readByte;
  boolean tag = false;
  if(Serial1.available() == tagLen){
    tag = true;
  }
  
  if(tag == true){
    while(Serial1.available()){
      readByte = Serial1.read();
      if(readByte != 13 && readByte != 10){
        newTag[i] = readByte;
        i++;
      }
      if(readByte == 10){
        tag = false;
      }
    }
  }
  if(strlen(newTag)==0)
  return 0;
  else
  return 1;
}

void access(){
  /*readTag();
  if(strlen(newTag)==0){
    return;
  }
  else{*/
  if(checkTag(newTag,addTag)>0){
    Serial.println("Swipe Card to give access");
    while(readTag()==0);
    for(int i=0;i<=tagIndx;i++)
    if(checkTag(newTag,knownTags[i])){
      Serial.println("Card already present");
      return;
    }
    ++tagIndx;
    Serial.println(tagIndx);
    for(int ind=0;ind<idLen;ind++)
    knownTags[tagIndx][ind]=newTag[ind];
  }
  else if(checkTag(newTag,delTag)>0){
    if(tagIndx==-1){
      Serial.println("No card saved");
      return;
    }
    Serial.println("Swipe Card to delete");
    while(readTag()==0);
    boolean match = false;
    int pos;
    for(int ct=0;ct<=tagIndx;ct++)
    if(checkTag(newTag,knownTags[ct])>0){
      match = true;
      pos = ct;
    }
    if(match == false){
      Serial.println("No card found");
      return;
    }
    else{
      for(int i=0;i<pos;i++)
      for(int j=0;j<idLen;j++)
      tempTags[i][j]=knownTags[i][j];
      for(int i=pos+1;i<tagIndx;i++)
      for(int j=0;j<idLen;j++)
      tempTags[i][j]=knownTags[i][j];
      --tagIndx;
      for(int i=0;i<tagIndx;i++)
      for(int j=0;j<idLen;j++)
      knownTags[i][j]=tempTags[i][j];
    }
  }
  else{
    //Serial.println(newTag);
    int total = 0;
    for(int ct=0 ; ct<kTags ; ct++){
      total += checkTag(newTag, knownTags[ct]);
    }
    if(total>0){
      Serial.println("Welcome");
    }
    else{
      Serial.println("access Denied");
    }
  }
}
