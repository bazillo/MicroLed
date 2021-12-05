#include "mbed.h"
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <vector>
#define MQTTCLIENT_QOS2 1
 
#include "MQTTmbed.h"
#include "MQTTClientMbedOs.h"
 
#include "neopixel.h"




NeoPixelOut npx(D11);
Pixel *strip;

int mode = 0; //1 - rainbowFlow, 2 - russian flag, 3 - rainbow, 4 - matrica, 5 - letovo, 6 - rainbow circle, 7 - cirles
int color = 0;
string text = "";

const int height = 7;
const int width = 7;
int mask[height][100 * width];
int colorMap[width];
int matrix[height][100 * width];
vector<vector<int>> curentMatrix(height,vector<int> (width,0));
int length = 0;
int curt = 0;
int distK = 4;
int russia[3] = {(1<<24)-1,255,1<<23};
int rainbowColors[7] = {8388608,16734720,16776960,65280,65530,255,16384250};
int ly = (1<<24) - 1 - 255;
int LetovoMatrix[7][7]= {
    {0  ,100,100,100,100,100,0  },
    {0  ,255,0  ,ly ,0  ,255,0  },
    {0  ,0  ,ly ,ly ,ly ,0  ,0},
    {0  ,ly ,ly ,0  ,ly ,ly ,0},
    {0  ,ly ,0  ,0  ,0  ,ly ,0},
    {0  ,0  ,255,0  ,255,0  ,0},
    {0  ,0  ,0  ,255,0  ,0  ,0}
};
int x = 0, y = 0;
int cirleC = 0;



int rainbow(int t){
    t %= 1000000;
    double wr = 0.05, wg = 0.05,wb = 0.05;
    int tr = 42, tg = 255, tb = 140;
    int r = min((int)(128 * (sin(wr * t + tr) + 1)),255);
    int g = min((int)(128 * (sin(wg * t + tg) + 1)),255);
    int b = min((int)(128 * (sin(wb * t + tb) + 1)),255);
    return (r<<16) + (g<<8) + b;
}

void matrica(){
    for(int i = height-1; i> 0; i--){
        for(int j = 0; j < width; j++){
            curentMatrix[i][j] = curentMatrix[i-1][j];
        }
    }
    
    for(int i = 0; i < width; i++){
        if(curentMatrix[0][i] == 0){
            curentMatrix[0][i] =  0x00FF00 * (rand()%11 == 0);
        }
        else if (curentMatrix[0][i] < 0x002000){
            curentMatrix[0][i] = 0;
        }
        else{
           curentMatrix[0][i] -= 0x002000;
        }
    }
}


void sendStrip() {
    curt++;
    switch (mode) {
    case 0:
        npx.normalize = false;
        break;
    case 1:
        npx.normalize = true;
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                int pos = i * height + j;
                strip[pos].hex = rainbow(distK * (((i%2?height-1-pos%height:pos%height) + (pos/height))) + curt);
            }
        }
        break;
    case 2:
        for(int i = 0; i < 3; i+= 1){
            for(int h = height * i /3; h < height *(i+1)/3; h++){
                for(int j = 0 ; j < width; j++){
                    strip[h * width + j].hex = russia[i];
                }
            }
        }
        break;
    case 3:
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                strip[i*width + j].hex = rainbowColors[((i%2?width-1-j:j) + curt/10)%7];
            }
        }
        break;
    
    case 4:
        npx.normalize = false;
        if(curt%6 == 0)matrica();
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                strip[i*width + j].hex = curentMatrix[i][i%2?width-1-j:j];
            }
        }
        break;
    case 5:
        npx.normalize = false;
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                strip[i*width + j].hex = LetovoMatrix[i][i%2?width-1-j:j];
            }
        }
        break;
    case 6:
        npx.normalize = false;
        int t = curt / 2;
        if( t%250== 0){
            x = rand()%7, y = rand()%7;
            while(x == 0 || y == 0 || x == height - 1 || y == width-1)
                x = rand()%7, y = rand()%7;
            }
        for(int i = 0 ; i < height; i++){
            for(int j = 0; j < width; j++){
                int ny =  i%2?width-1-j:j;
                strip[i*width + j].hex = rainbow( (distK*(abs(x - i) + abs(y - ny)) + t));
            }
        }
        break;
    }
    if(mode == 7) {
        npx.normalize = false;
        int t = curt / 2;
        if( t%70== 0){
            cirleC = rand()%7;
            x = rand()%7, y = rand()%7;
            while(x == 0 || y == 0 || x == height - 1 || y == width-1)
                x = rand()%7, y = rand()%7;
            }
        for(int i = 0 ; i < height; i++){
            for(int j = 0; j < width; j++){
                int ny =  i%2?width-1-j:j;
                strip[i*width + j].hex = (abs(x - i) + abs(y - ny) == (t/10)%7)*rainbowColors[cirleC];;
            }
        }
    }
    

    npx.send(strip,width*height);
}


void setByVector(vector<int> &a){
    for(int i = 0; i < a.size();i++){
        strip[i].hex = a[i];
    }
}



void messageArrived(MQTT::MessageData& md)
{
    // printf("Got it\n");
    MQTT::Message &message = md.message;
    // printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s Payloadlen %d\r\n", message.payloadlen, (char*)message.payload,message.payloadlen);
    stringstream inn((char*)message.payload);
    string command;
    inn >> command;
    mode = 0;
    for(int i = 0 ; i < width*height; i++) strip[i].hex=0;
    if(command == "setText"){
        text = "";
        string t;
        while (inn >> t) {
            text += t;
        }
        printf("Text setted\r\n");
    }else if (command == "setColor") {
        inn >> color;
        printf("Color setted\r\n");
    }else if(command == "setMode"){
        inn >> mode;
        printf("Mode setted\r\n");
    }else if (command == "setMatrix") {
        length = 0;
        int pos = 0;
        while(((char*)message.payload)[pos] !=' ') pos++;
        vector<int> parser;
        int a = 0;
        for(int i = pos + 1; i < message.payloadlen;i++){
            char c = ((char*)message.payload)[i];
            if(c == ' ') {
                parser.push_back(a);
                a = 0;    
            }
            if('0' <= c && c <= '9') a= a*10 + (c - '0');
        }
        parser.push_back(a);
    
        setByVector(parser);
    }
    else{
        printf("Unknown command\r\n");
    }
}
void start(){
    for(int i = 0; i < 7; i++){
        for(int j = 0; j  < width * height; j++){
            strip[j].hex = rainbowColors[i];
        }
        npx.send(strip, height * width);
        thread_sleep_for(100);
    }
}
 
TCPSocket socket;
MQTTClient client(&socket);
WiFiInterface *wifi;
char* topic = "zoro_k@mail.ru/led";
int main()
{   
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    if (true){ //настраиваем WIFI и MQTT

        wifi = WiFiInterface::get_default_instance();
        if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
        }
        printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
        // Error handling 
        int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
        if (ret != -3015 && ret != 0) {
            printf("\nConnection error: %d\n", ret);
            return -1;
        }
        printf("Success\n\n");
        
        float version = 0.6;
        
    
        SocketAddress a;
        char* hostname = "maqiatto.com";
        wifi->gethostbyname(hostname, &a);
        int port = 1883;
        a.set_port(port);
        
        printf("Connecting to %s:%d\r\n", hostname, port);
    
        socket.open(wifi);
        printf("Opened socket\n\r");
        int rc = socket.connect(a);
        if (rc != 0)
            printf("rc from TCP connect is %d\r\n", rc);
        printf("Connected socket\n\r");
    
        
        data.MQTTVersion = 3;
        data.clientID.cstring = "LED";
        data.username.cstring = "zoro_k@mail.ru";
        data.password.cstring = "Rbhbkk2004";
        if ((rc = client.connect(data)) != 0)
            printf("rc from MQTT connect is %d\r\n", rc);
    
        if ((rc = client.subscribe(topic, MQTT::QOS0, messageArrived)) != 0)
            printf("rc from MQTT subscribe is %d\r\n", rc);
    }
    
    npx.global_scale = 0.40f; 
    npx.normalize = false;
    strip = new Pixel[width*height];
    start();
    for(int i = 0 ; i < width*height; i++) strip[i].hex=0;
    while (true){
        client.yield(10);
        if (!client.isConnected()) {
            client.connect(data);
            client.subscribe(topic, MQTT::QOS0, messageArrived);
        }
        sendStrip();
    }

}
 
