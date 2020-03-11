/*
 * https://www.moonmile.net/blog/archives/9229　を参考に、M5StickからSlackにポストするルーチン
 * 
 * 前提：
 * ・WiFi接続が完了していること。
 * ・SDカードにslackhook.txtを置く。（その中に改行無しで送信対象のWebHookURLを入れる）
 * 
 */
//#include <HTTPClient.h>   //メインのinoに入っていれば不要

////////////////////////////////

// Slack にログをポストするかどうか
boolean SLACK_POST = true;

//SlackのhookURL入りのファイル
const char* Slackfname = "/slackhook.txt";

HTTPClient http;
String services;

const char *server = "hooks.slack.com";
//char *json = "{\"text\":\"ENV_LOG START\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}";

const char* slack_root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n" ;
   

void slack_init(){
    //Slack
    unsigned int cnt = 0;
    char data[128];
    char *str;
    File fp;
    
    //slackhook.txt 読み込み
    fp = SD.open(Slackfname, FILE_READ);
    if(fp == false){  SLACK_POST = false;  }
    
    if(SLACK_POST){
        while(fp.available()){
          data[cnt++] = fp.read();
        }
        close(fp);
    
        String s = (String)data;
    
        int l = s.indexOf("/services/");
        //M5.Lcd.println(l);  // 23のはず
      
        //M5.Lcd.print(s.substring(l));
      
        //String services = s.substring(l);
        services = s.substring(l);
        M5.Lcd.print(services);


        //sprintf(json,"{\"text\":\"Slack Send START\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}");
        char *json = "{\"text\":\"Slack Send START\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}";
        
        // Slack Post
        http.begin( server, 443, services, slack_root_ca );
        http.addHeader("Content-Type", "application/json" );
        http.POST((uint8_t*)json, strlen(json));
        M5.Lcd.println("post hooks.slack.com");
        http.end();
    }  

}

void slack_post(char *json_buf){
        //Slack Post
        if( SLACK_POST ){
            //sprintf(jsonnow,"{\"text\":\"Temperatura: %2.2f*C  Humedad: %0.2f%%  Pressure: %0.2fhPa\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}", tmp, hum, pressure);
            Serial.printf(json_buf);
        
            // Slack Post
            http.begin( server, 443, services, slack_root_ca );
            http.addHeader("Content-Type", "application/json" );
            http.POST((uint8_t*)json_buf, strlen(json_buf));
            Serial.printf("\n post hooks.slack.com\n");
            http.end();
        }

}
