#include <jni.h>
#include <string>
#include <reader.h>
#include "JniUtils.h"
#include "web_task.h"
#include <pthread.h>
#include <stdlib.h>


CURLcode curl_get_req(const std::string &url, std::string &response)
;
size_t req_reply(void *ptr, size_t size, size_t nmemb, void *stream);

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);

void* threadRun(void* arg);

using namespace std;

//http://www.weather.com.cn/data/sk/101110101.html
//{"weatherinfo":{"city":"深圳","cityid":"101280601","temp":"21","WD":"南风","WS":"1级","SD":"31%","WSE":"1","time":"17:05","isRadar":"1","Radar":"JC_RADAR_AZ9755_JB","njd":"暂无实况","qy":"1014","rain":"0"}}

extern "C"
jstring
Java_com_ldlywt_jnihttp_MainActivity_httpFromJNI(JNIEnv *env, jobject /* this */) {

/*    //POST请求,举例来说
    string url = "http://www.weather.com.cn/user/add";
    WebTask task;
    task.SetUrl(url.c_str());
    task.SetConnectTimeout(5);
    task.AddPostString("username", username);
    task.AddPostString("password", password);
    task.AddPostString("email", email);*/

    //GET请求
    string url = "http://www.weather.com.cn/data/sk/101280601.html";
    WebTask task;
    task.SetUrl(url.c_str());
    task.SetConnectTimeout(5);
    task.DoGetString();
    if (task.WaitTaskDone() == 0) {
        //请求服务器成功
        string jsonResult = task.GetResultString();
        LOGI("返回的json数据是：%s\n", jsonResult.c_str());
        Json::Reader reader;
        Json::Value root;

    //从字符串中读取数据
        if (reader.parse(jsonResult, root)) {
            /*//根节点
            Json::Value weatherinfo = root["weatherinfo"];
            string js1 = weatherinfo["city"].asString();
            LOGI("根节点解析: %s\n", js1.c_str());*/
            //读取子节点信息
            string city = root["weatherinfo"]["city"].asString();
            string temp = root["weatherinfo"]["temp"].asString();
            string WD = root["weatherinfo"]["WD"].asString();
            string WS = root["weatherinfo"]["WS"].asString();
            string time = root["weatherinfo"]["time"].asString();
            string result = "城市：" + city + "\n温度："+ temp+ "\n风向：" + WD + "\n风力："+ WS + "\n时间：" + time;
                    return Str2Jstring(env, result.c_str());
        }
    } else {
        LOGI("网络连接失败\n");
        return env->NewStringUTF("网络连接失败！");
    }
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_ldlywt_jnihttp_MainActivity_get(JNIEnv *env, jobject instance) {

    // TODO
    const char *fileName = "/sdcard/jni2.mp3";
    getUrl(fileName);

//    string getUrlStr = "http://www.baidu.com";
//    string getResponseStr;
//    auto res = curl_get_req(getUrlStr, getResponseStr);
//    if (res != CURLE_OK){
//        LOGI("请求出错");
//    } else{
//        LOGI("请求结果:%s", getResponseStr.c_str());
//    }

    return env->NewStringUTF("hello");
}
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
//    FILE* fp = NULL;
//    fp = fopen("/sdcard/jni.apk", "ab+");
//    size_t nWrite = fwrite(ptr, size, nmemb, fp);
//    fclose(fp);
//    return nWrite;
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
bool getUrl(const char *filename)
{
    CURL *curl;
    CURLcode res;
    FILE *fp;
    if ((fp = fopen(filename, "wb")) == NULL)  // 返回结果用文件存储
    {
        LOGI("文件不可读写");
        return false;
    }
    curl = curl_easy_init();    // 初始化
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(curl, CURLOPT_URL,"http://fs.w.kugou.com/201904251056/7274014571a0cbff50caa5abdbb4d320/G157/M00/12/11/3Q0DAFy9uBuAYaNkAEj94uwiMZs807.mp3");
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); //将返回的html主体数据输出到fp指向的文件
//        curl_easy_setopt(curl, CURLOPT_HEADERDATA, fp); //将返回的http头输出到fp指向的文件
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30); // set transport and time out time
        res = curl_easy_perform(curl);   // 执行
        if (res != 0) {
//            LOGI("结果:%d", res);
        }
        LOGI("结果:%d", res);
        curl_easy_cleanup(curl);
        fclose(fp);
        return true;
    }
}


CURLcode curl_get_req(const std::string &url, std::string &response)
{
    // init curl
    CURL *curl = curl_easy_init();
    // res code
    CURLcode res;
    if (curl)
    {
        // set params
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // url
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_HEADER, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3); // set transport and time out time
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
        // start req
        res = curl_easy_perform(curl);
    }
    // release curl
    curl_easy_cleanup(curl);
    return res;
}


size_t req_reply(void *ptr, size_t size, size_t nmemb, void *stream)
{
    LOGI("----->reply");
    string *str = (string*)stream;
    LOGI("%s", str);
    (*str).append((char*)ptr, size*nmemb);
    return size * nmemb;
}

struct MyArg{
    char *path;//文件绝对路径
    char *str;//额外字符
}myArg;

void* threadRun(void* arg){
    struct MyArg *c = (MyArg *) arg;

    FILE *fp = fopen(c->path, "r");
    if(fp == NULL){
        LOGI("文件不可读/读取失败");
    }
    char strLine[1024];
    while(!feof(fp)){
        fgets(strLine, 1024, fp);
        if (strstr(strLine, "输入法")){
            LOGI("包含字符串===>%s", strLine);
        }
        LOGI("===>%s", strLine);
    }
    fclose(fp);

    const char *c1 = "111";
    const char *c2 = "111";
    if(strcmp(c1, c2) == 0){
        LOGI("字符完全相同");
    } else{
        LOGI("字符不完全相同");
    }

    LOGI("参数:%s", c->str);
    for (int i = 0; i < 10; ++i) {
        LOGI("循环%d", i);
        if(i == 5){
//            break;
            pthread_exit(NULL);
        }
    }
    return NULL;
//    pthread_exit((void *) 1);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_ldlywt_jnihttp_MainActivity_file(JNIEnv *env, jobject instance, jstring string_) {

    pthread_t thread;

    const char *string = env->GetStringUTFChars(string_, 0);

    struct MyArg myArg1;
    myArg1.path = (char *) string;
    myArg1.str = (char *) env->GetStringUTFChars(string_, JNI_FALSE);

    void* arg = &myArg1;

    int i = pthread_create(&thread, NULL, threadRun, arg);

    LOGI("结果:%d", i);
//    void* status;
    pthread_join(thread, NULL);

    env->ReleaseStringUTFChars(string_, string);
    return true;
}