/*
@module  wlan
@summary wifi操作库
@version 1.0
@date    2020.03.30
*/
#include "luat_base.h"
#include "luat_timer.h"
#include "luat_malloc.h"

#include <wifi_mgmr_ext.h>
#include <bl_wifi.h>
#include <hal_wifi.h>
#include <aos/yloop.h>
#include <FreeRTOS.h>
#include <task.h>
#include <hal_sys.h>
#include <aos/kernel.h>

/*
获取wifi模式
@api wlan.getMode(dev)
@string  设备名称,字符串或数值, 可选值0/1, "wlan0","wlan1". 默认"wlan0"
@return int 模式wlan.NONE, wlan.STATION, wlan.AP
@usage
-- 获取wlan0的当前模式
local m = wlan.getMode("wlan0")
*/
static int l_wlan_get_mode(lua_State *L) {
    char* devname = "wlan0";
    if (lua_gettop(L) != 0) {
        if (lua_isstring(L, 1)) {
            devname = lua_tostring(L, 1);
        }
        else if (lua_isinteger(L, 1)) {
            switch (lua_tointeger(L, 1))
            {
            case 1:
                devname = "wlan1";
                break;
            default:
                break;
            }
        }
    }
    int mode = 0;// = rt_wlan_get_mode(devname);
    lua_pushinteger(L, mode);
    return 1;
}

/*
设置wifi模式,通常不需要设置
@api wlan.setMode(dev, mode)
@string  设备名称,字符串或数值, 可选值0/1, "wlan0","wlan1". 默认"wlan0"
@int 模式wlan.NONE, wlan.STATION, wlan.AP
@return int   设置成功与否,通常不检查
@usage 
-- 将wlan设置为wifi客户端模式
wlan.setMode("wlan0",wlan.STATION) 
*/
static int l_wlan_set_mode(lua_State *L) {
    char* devname = "wlan0";
    if (lua_gettop(L) != 0) {
        if (lua_isstring(L, 1)) {
            devname = lua_tostring(L, 1);
        }
        else if (lua_isinteger(L, 1)) {
            if (lua_tointeger(L, 1) == 1) {
                devname = "wlan1";
            }
        }
    }
    int mode = luaL_checkinteger(L, 2);
    int re = 0;// = rt_wlan_set_mode(devname, mode);
    lua_pushinteger(L, re);
    return 1;
}

typedef struct join_info
{
    char ssid[64];
    char passwd[64];
} join_info_t;
static join_info_t jinfo = {0};


static void wifi_sta_connect(char *ssid, char *password)
{
    wifi_interface_t wifi_interface;

    wifi_interface = wifi_mgmr_sta_enable();
    wifi_mgmr_sta_connect(wifi_interface, ssid, password, NULL, NULL, 0, 0);
}

/*
连接wifi,成功启动联网线程不等于联网成功!!
@api wlan.connect(ssid,password)
@string  ssid  wifi的SSID
@string password wifi的密码,可选
@return boolean   如果正常启动联网线程,无返回值,否则返回出错信息. 
@usage 
-- 连接到uiot,密码1234567890
wlan.connect("uiot", "1234567890")
*/
static int l_wlan_connect(lua_State *L) {
    // 更新参数
    size_t len;
    const char* _ssid = luaL_checklstring(L, 1, &len);
    strncpy(jinfo.ssid, _ssid, len);
    jinfo.ssid[len] = 0x00;
    if (lua_isstring(L, 2)) {
        const char* _passwd = luaL_checklstring(L, 2, &len);
        strncpy(jinfo.passwd, _passwd, len);
        jinfo.passwd[len] = 0x00;
    }
    else {
        jinfo.passwd[0] = 0x00;
    }
    wifi_sta_connect(jinfo.ssid, jinfo.passwd);
    return 0;
}

/*
断开wifi
@api wlan.disconnect()
@return boolean 成功返回true,否则返回false
@usage
-- 断开wifi连接 
wlan.disconnect()
*/
static int l_wlan_disconnect(lua_State *L) {
    int s_code;
    wifi_mgmr_status_code_get(&s_code);

    if (s_code != WIFI_STATE_DISCONNECT) {
        wifi_mgmr_sta_disconnect();
        lua_pushboolean(L, 1);
    }
    else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

/*
是否已经连上wifi网络
@api wlan.connected()
@return boolean 已连接返回true0,未连接返回false
@usage 
-- 连上wifi网络,只代表密码正确, 不一定拿到了ip
wlan.connected()
*/
static int l_wlan_connected(lua_State *L) {
    int s_code;
    wifi_mgmr_status_code_get(&s_code);
    lua_pushboolean(L, s_code == WIFI_STATE_DISCONNECT ? 0 : 1);
    return 1;
}

/*
设置或查询wifi station是否自动连接
@api wlan.autoreconnect(enable)
@int 传入1启用自动连接(自动重连wifi), 传入0关闭. 不传这个参数就是查询
@return int 已启用自动连接(自动重连wifi)返回1, 否则返回0
@usage 
-- 查询自动连接的设置
wlan.autoreconnect()
@usage
-- 设置自动连接
wlan.autoreconnect(1)
*/
static int l_wlan_autoreconnect(lua_State *L) {
    static int autoconnect = 0;
    if (lua_gettop(L) > 0) {
        if( luaL_checkinteger(L, 1) ) {
            wifi_mgmr_sta_autoconnect_enable();
            autoconnect = 1;
        } 
        else
        {
            wifi_mgmr_sta_autoconnect_disable();
            autoconnect = 0;
        }
    }
    lua_pushboolean(L, autoconnect);
    return 1;
}

/*
开始扫网,通常配合wlan.scanResult使用
@api wlan.scan()
@return boolean 启动结果,一般为true
@usage 
-- 扫描并查询结果
wlan.scan()
sys.waitUntil("WLAN_SCAN_DONE", 30000)
local re = wlan.scanResult()
for i in ipairs(re) do
    log.info("wlan", "info", re[i].ssid, re[i].rssi)
end
*/
static int l_wlan_scan(lua_State *L) {
    lua_pushboolean(L, 0);
    return 1;
}

/*
获取扫网结果,需要先执行wlan.scan,并等待WLAN_SCAN_DONE事件
@api wlan.scanResult(num)
@int 最大结果数量,默认50
@return table 扫描结果的数组
@usage 
-- 扫描并查询结果
wlan.scan()
sys.waitUntil("WLAN_SCAN_DONE", 30000)
local re = wlan.scanResult()
for i in ipairs(re) do
    log.info("wlan", "info", re[i].ssid, re[i].rssi)
end
*/
static int l_wlan_scan_get_result(lua_State *L) {
    int num = luaL_optinteger(L, 1, 20);                        /* 查询扫描结果数量 */
    // struct rt_wlan_scan_result *result = NULL;// = rt_wlan_scan_get_result();
    // if (result) {
    //     LOG_I("wlan_scan_get_result >> %ld", result->num);
    // }
    lua_createtable(L, num, 0);
    // if (result && result->num > 0) {
    //     if (result->num < num) {
    //         num = result->num;
    //     }
    //     for (size_t i = 0; i < num; i++)
    //     {
    //         struct rt_wlan_info info = result->info[i];
    //         lua_pushinteger(L, i+1);

    //         // local info = {}
    //         lua_createtable(L, 0, 8);

    //         // info.ssid = xxx
    //         lua_pushliteral(L, "ssid");
    //         lua_pushlstring(L, info.ssid.val, info.ssid.len);
    //         lua_settable(L, -3);

    //         // info.rssi = xxx
    //         lua_pushliteral(L, "rssi");
    //         lua_pushinteger(L, info.rssi);
    //         lua_settable(L, -3);

    //         // info.security = xxx
    //         lua_pushliteral(L, "security");
    //         lua_pushinteger(L, info.security);
    //         lua_settable(L, -3);

    //         // info.channel = xxx
    //         lua_pushliteral(L, "channel");
    //         lua_pushinteger(L, info.channel);
    //         lua_settable(L, -3);

    //         // info.band = xxx
    //         lua_pushliteral(L, "band");
    //         lua_pushinteger(L, info.band);
    //         lua_settable(L, -3);

    //         // info.bssid = xxx
    //         lua_pushliteral(L, "bssid");
    //         lua_pushlstring(L, info.bssid, 6);
    //         lua_settable(L, -3);

    //         // info.hidden = xxx
    //         lua_pushliteral(L, "hidden");
    //         lua_pushinteger(L, info.hidden);
    //         lua_settable(L, -3);

    //         // re[i+1] = info
    //         lua_settable(L, -3);
    //     }
    // }
    return 1;
}

/*
获取mac地址
@function wlan.get_mac()
@return string 长度为12的HEX字符串,如果不存在就返回值nil
@usage 
-- 获取MAC地址
log.info("wlan", "mac addr", wlan.get_mac())
*/
static int l_wlan_get_mac(lua_State *L) {
    uint8_t mac[6];
    char buff[14];
    mac[0] = 0x00;
    wifi_mgmr_sta_mac_get(mac);
    if (mac[0] != 0x00) {
        sprintf(buff, 14, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        lua_pushlstring(L, buff, 12);
        return 1;
    }
    return 0;
}

/*
获取mac地址,raw格式
@api wlan.get_mac_raw()
@return string 6字节的mac地址串
@usage 
-- 查询mac地址, 二进制模式
local mac_raw  = wlan.get_mac_raw()
if mac_raw then
    log.info("wlan", "mac addr", mac_raw:toHex())
end
*/
static int l_wlan_get_mac_raw(lua_State *L) {
    uint8_t mac[6];
    mac[0] = 0x00;
    wifi_mgmr_sta_mac_get(mac);
    if (mac[0] != 0x00) {
        lua_pushlstring(L, (const char*)mac, 6);
        return 1;
    }
    return 0;
}

/*
wifi是否已经获取ip
@api wlan.ready()
@return boolean 已经有ip返回true,否则返回false
@usage 
-- 查询是否已经wifi联网
if wlan.ready() then
    log.info("wlan", "wifi ok", "Let's Rock!")
end
*/
static int l_wlan_ready(lua_State *L) {
    lua_pushboolean(L, 1);
    return 1;
}

// static int l_wlan_handler(lua_State* L, void* ptr) {
//     rtos_msg_t* msg = (rtos_msg_t*)lua_topointer(L, -1);
//     int event = msg->arg1;

//     lua_getglobal(L, "sys_pub");
//     if (lua_isnil(L, -1)) {
//         lua_pushinteger(L, 0);
//         return 1;
//     }

//     switch (event)
//     {
//     case CODE_WIFI_ON_INIT_DONE:
//     {
//         printf("[APP] [EVT] INIT DONE %lld\r\n", aos_now_ms());
//         wifi_mgmr_start_background(&conf);

//         lua_pushstring(L, "WLAN_READY");
//         lua_call(L, 1, 0);
//         // 额外发送一个通用事件 NET_READY
//         lua_getglobal(L, "sys_pub");
//         lua_pushstring(L, "NET_READY");
//         lua_call(L, 1, 0);
//     }
//     break;
//     case CODE_WIFI_ON_MGMR_DONE:
//     {
//         printf("[APP] [EVT] MGMR DONE %lld\r\n", aos_now_ms());
//     }
//     break;
//     case CODE_WIFI_ON_MGMR_DENOISE:
//     {
//         printf("[APP] [EVT] Microwave Denoise is ON %lld\r\n", aos_now_ms());
//     }
//     break;
//     case CODE_WIFI_ON_SCAN_DONE:
//     {
//         printf("[APP] [EVT] SCAN Done %lld\r\n", aos_now_ms());
//         wifi_mgmr_cli_scanlist();
//     }
//     break;
//     case CODE_WIFI_ON_SCAN_DONE_ONJOIN:
//     {
//         printf("[APP] [EVT] SCAN On Join %lld\r\n", aos_now_ms());
//     }
//     break;
//     case CODE_WIFI_ON_DISCONNECT:
//     {
//         printf("[APP] [EVT] disconnect %lld, Reason: %s\r\n",
//                aos_now_ms(),
//                wifi_mgmr_status_code_str(event->value));
//     }
//     break;
//     case CODE_WIFI_ON_CONNECTING:
//     {
//         printf("[APP] [EVT] Connecting %lld\r\n", aos_now_ms());
//     }
//     break;
//     case CODE_WIFI_CMD_RECONNECT:
//     {
//         printf("[APP] [EVT] Reconnect %lld\r\n", aos_now_ms());
//     }
//     break;
//     case CODE_WIFI_ON_CONNECTED:
//     {
//         printf("[APP] [EVT] connected %lld\r\n", aos_now_ms());
//     }
//     break;
//     case CODE_WIFI_ON_PRE_GOT_IP:
//     {
//         printf("[APP] [EVT] connected %lld\r\n", aos_now_ms());
//     }
//     break;
//     case CODE_WIFI_ON_GOT_IP:
//     {
//         printf("[APP] [EVT] GOT IP %lld\r\n", aos_now_ms());
//         printf("[SYS] Memory left is %d Bytes\r\n", xPortGetFreeHeapSize());
//     }
//     break;
//     case CODE_WIFI_ON_EMERGENCY_MAC:
//     {
//         printf("[APP] [EVT] EMERGENCY MAC %lld\r\n", aos_now_ms());
//         hal_reboot(); //one way of handling emergency is reboot. Maybe we should also consider solutions
//     }
//     break;
//     case CODE_WIFI_ON_PROV_SSID:
//     {
//         printf("[APP] [EVT] [PROV] [SSID] %lld: %s\r\n",
//                aos_now_ms(),
//                event->value ? (const char *)event->value : "UNKNOWN");
//         if (ssid)
//         {
//             vPortFree(ssid);
//             ssid = NULL;
//         }
//         ssid = (char *)event->value;
//     }
//     break;
//     case CODE_WIFI_ON_PROV_BSSID:
//     {
//         printf("[APP] [EVT] [PROV] [BSSID] %lld: %s\r\n",
//                aos_now_ms(),
//                event->value ? (const char *)event->value : "UNKNOWN");
//         if (event->value)
//         {
//             vPortFree((void *)event->value);
//         }
//     }
//     break;
//     case CODE_WIFI_ON_PROV_PASSWD:
//     {
//         printf("[APP] [EVT] [PROV] [PASSWD] %lld: %s\r\n", aos_now_ms(),
//                event->value ? (const char *)event->value : "UNKNOWN");
//         if (password)
//         {
//             vPortFree(password);
//             password = NULL;
//         }
//         password = (char *)event->value;
//     }
//     break;
//     case CODE_WIFI_ON_PROV_CONNECT:
//     {
//         printf("[APP] [EVT] [PROV] [CONNECT] %lld\r\n", aos_now_ms());
//         printf("connecting to %s:%s...\r\n", ssid, password);
//         wifi_sta_connect(ssid, password);
//     }
//     break;
//     case CODE_WIFI_ON_PROV_DISCONNECT:
//     {
//         printf("[APP] [EVT] [PROV] [DISCONNECT] %lld\r\n", aos_now_ms());
//     }
//     break;
//     case CODE_WIFI_ON_AP_STA_ADD:
//     {
//         printf("[APP] [EVT] [AP] [ADD] %lld, sta idx is %lu\r\n", aos_now_ms(), (uint32_t)event->value);
//     }
//     break;
//     case CODE_WIFI_ON_AP_STA_DEL:
//     {
//         printf("[APP] [EVT] [AP] [DEL] %lld, sta idx is %lu\r\n", aos_now_ms(), (uint32_t)event->value);
//     }
//     break;
//     default:
//     {
//         printf("[APP] [EVT] Unknown code %u, %lld\r\n", event->code, aos_now_ms());
//         /*nothing*/
//     }
//     }

    // switch (event)
    // {
    // case RT_WLAN_EVT_READY: // 网络就绪
    //     lua_pushstring(L, "WLAN_READY");
    //     lua_call(L, 1, 0);
    //     // 额外发送一个通用事件 NET_READY
    //     lua_getglobal(L, "sys_pub");
    //     lua_pushstring(L, "NET_READY");
    //     lua_call(L, 1, 0);
    //     break;
    // case RT_WLAN_EVT_SCAN_DONE: // 扫描完成
    //     lua_pushstring(L, "WLAN_SCAN_DONE");
    //     lua_call(L, 1, 0);
    //     break;
    // case RT_WLAN_EVT_STA_CONNECTED: // 连上wifi路由器/热点,但还没拿到ip
    //     lua_pushstring(L, "WLAN_STA_CONNECTED");
    //     lua_pushinteger(L, 1);
    //     lua_call(L, 2, 0);
    //     break;
    // case RT_WLAN_EVT_STA_CONNECTED_FAIL: // 没有连上wifi路由器/热点,通常是密码错误
    //     lua_pushstring(L, "WLAN_STA_CONNECTED");
    //     lua_pushinteger(L, 0);
    //     lua_call(L, 2, 0);
    //     break;
    // case RT_WLAN_EVT_STA_DISCONNECTED: // 从wifi路由器/热点断开了
    //     lua_pushstring(L, "WLAN_STA_DISCONNECTED");
    //     lua_call(L, 1, 0);
    //     break;
    // case RT_WLAN_EVT_AP_START:
    //     lua_pushstring(L, "WLAN_AP_START");
    //     lua_call(L, 1, 0);
    //     break;
    // case RT_WLAN_EVT_AP_STOP:
    //     lua_pushstring(L, "WLAN_AP_STOP");
    //     lua_call(L, 1, 0);
    //     break;
    // case RT_WLAN_EVT_AP_ASSOCIATED:
    //     lua_pushstring(L, "WLAN_AP_ASSOCIATED");
    //     lua_call(L, 1, 0);
    //     break;
    // case RT_WLAN_EVT_AP_DISASSOCIATED:
    //     lua_pushstring(L, "WLAN_AP_DISASSOCIATED");
    //     lua_call(L, 1, 0);
    //     break;
    
    // default:
    //     break;
    // }
//     lua_pushinteger(L, 0);
//     return 1;
// }

static wifi_conf_t conf = {
    .country_code = "CN",
};

// 注册回调
static void event_cb_wifi_event(input_event_t *event, void *private_data)
{
    static char *ssid;
    static char *password;
    
    // rtos_msg_t msg;
    // LOG_I("wlan event -> %d", event);
    // msg.handler = l_wlan_handler;
    // msg.ptr = NULL;
    // msg.arg1 = event->code;
    // msg.arg2 = 0;

    // luat_msgbus_put(&msg, 1);

    switch (event->code)
    {
    case CODE_WIFI_ON_INIT_DONE:
    {
        printf("[APP] [EVT] INIT DONE %lld\r\n", aos_now_ms());
        wifi_mgmr_start_background(&conf);
    }
    break;
    case CODE_WIFI_ON_MGMR_DONE:
    {
        printf("[APP] [EVT] MGMR DONE %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_MGMR_DENOISE:
    {
        printf("[APP] [EVT] Microwave Denoise is ON %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_SCAN_DONE:
    {
        printf("[APP] [EVT] SCAN Done %lld\r\n", aos_now_ms());
        wifi_mgmr_cli_scanlist();
    }
    break;
    case CODE_WIFI_ON_SCAN_DONE_ONJOIN:
    {
        printf("[APP] [EVT] SCAN On Join %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_DISCONNECT:
    {
        printf("[APP] [EVT] disconnect %lld, Reason: %s\r\n",
               aos_now_ms(),
               wifi_mgmr_status_code_str(event->value));
    }
    break;
    case CODE_WIFI_ON_CONNECTING:
    {
        printf("[APP] [EVT] Connecting %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_CMD_RECONNECT:
    {
        printf("[APP] [EVT] Reconnect %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_CONNECTED:
    {
        printf("[APP] [EVT] connected %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_PRE_GOT_IP:
    {
        printf("[APP] [EVT] connected %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_GOT_IP:
    {
        printf("[APP] [EVT] GOT IP %lld\r\n", aos_now_ms());
        printf("[SYS] Memory left is %d Bytes\r\n", xPortGetFreeHeapSize());
    }
    break;
    case CODE_WIFI_ON_EMERGENCY_MAC:
    {
        printf("[APP] [EVT] EMERGENCY MAC %lld\r\n", aos_now_ms());
        hal_reboot(); //one way of handling emergency is reboot. Maybe we should also consider solutions
    }
    break;
    case CODE_WIFI_ON_PROV_SSID:
    {
        printf("[APP] [EVT] [PROV] [SSID] %lld: %s\r\n",
               aos_now_ms(),
               event->value ? (const char *)event->value : "UNKNOWN");
        if (ssid)
        {
            vPortFree(ssid);
            ssid = NULL;
        }
        ssid = (char *)event->value;
    }
    break;
    case CODE_WIFI_ON_PROV_BSSID:
    {
        printf("[APP] [EVT] [PROV] [BSSID] %lld: %s\r\n",
               aos_now_ms(),
               event->value ? (const char *)event->value : "UNKNOWN");
        if (event->value)
        {
            vPortFree((void *)event->value);
        }
    }
    break;
    case CODE_WIFI_ON_PROV_PASSWD:
    {
        printf("[APP] [EVT] [PROV] [PASSWD] %lld: %s\r\n", aos_now_ms(),
               event->value ? (const char *)event->value : "UNKNOWN");
        if (password)
        {
            vPortFree(password);
            password = NULL;
        }
        password = (char *)event->value;
    }
    break;
    case CODE_WIFI_ON_PROV_CONNECT:
    {
        printf("[APP] [EVT] [PROV] [CONNECT] %lld\r\n", aos_now_ms());
        printf("connecting to %s:%s...\r\n", ssid, password);
        wifi_sta_connect(ssid, password);
    }
    break;
    case CODE_WIFI_ON_PROV_DISCONNECT:
    {
        printf("[APP] [EVT] [PROV] [DISCONNECT] %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_AP_STA_ADD:
    {
        printf("[APP] [EVT] [AP] [ADD] %lld, sta idx is %lu\r\n", aos_now_ms(), (uint32_t)event->value);
    }
    break;
    case CODE_WIFI_ON_AP_STA_DEL:
    {
        printf("[APP] [EVT] [AP] [DEL] %lld, sta idx is %lu\r\n", aos_now_ms(), (uint32_t)event->value);
    }
    break;
    default:
    {
        printf("[APP] [EVT] Unknown code %u, %lld\r\n", event->code, aos_now_ms());
        /*nothing*/
    }
    }
}

static void cmd_stack_wifi(char *buf, int len, int argc, char **argv)
{
    /*wifi fw stack and thread stuff*/
    static uint8_t stack_wifi_init = 0;

    if (1 == stack_wifi_init)
    {
        puts("Wi-Fi Stack Started already!!!\r\n");
        return;
    }
    stack_wifi_init = 1;

    hal_wifi_start_firmware_task();
    /*Trigger to start Wi-Fi*/
    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);
}

static void reg_wlan_callbacks(void) {
    aos_register_event_filter(EV_WIFI, event_cb_wifi_event, NULL);
    cmd_stack_wifi(NULL, 0, 0, NULL);
}

// ----------------------------
//-----------------------------
// static int luat_PW_msghandler(lua_State *L, void* ptr) {
//     lua_getglobal(L, "sys_pub");
//     if (!lua_isnil(L, -1)) {
//         lua_pushstring(L, "WLAN_PW_RE");
//         if (ptr == RT_NULL) {
//             lua_call(L, 1, 0);
//         }
//         else {
//             lua_pushstring(L, jinfo.ssid);
//             lua_pushstring(L, jinfo.passwd);
//             lua_call(L, 3, 0);
//         }
//     }
//     // 给rtos.recv方法返回个空数据
//     lua_pushinteger(L, 0);
//     return 1;
// }

// static void _PW_callback(int state, unsigned char *_ssid, unsigned char *_passwd) {
//     LOG_I("oneshot/airkiss callback state=%ld", state);
//     if (_ssid != RT_NULL) {
//         LOG_I("oneshot/airkiss ssid %s", _ssid);
//     }
//     if (_passwd != RT_NULL) {
//         LOG_I("oneshot/airkiss key %s", _passwd);
//     }
//     rt_memset(&jinfo, 0, sizeof(struct join_info));
//     if (state == 0) {
//         rt_strncpy(jinfo.ssid, _ssid, rt_strlen(_ssid));
//         if (_passwd)
//         {
//             rt_strncpy(jinfo.passwd, _passwd, rt_strlen(_passwd));
//         }
//     }
    
//     // 发送msgbus消息
//     rtos_msg_t msg;
//     msg.handler = luat_PW_msghandler;
//     msg.ptr = state == 0 ? (void*)1 : RT_NULL;
//     luat_msgbus_put(&msg, 1);
// }

//--------------------------
// 联盛德的oneshot配网, 需要较多固定内存,默认不启用
#ifdef WM_USING_ONESHOT
static int32_t oneshot_autojoin = 0;
static int32_t oneshot_re;

/*
启动配网过程,支持UDP/SOCKET/APWEB配网
@api wlan.oneShotStart(mode,ssid,passwd)
@int 配网模式: 0-UDP配网, 1-SOCKET配网, 2-AP网页配网
@string AP网页配网时的SSID,默认值为luatos
@string AP网页配网时的密钥,默认值为12345678
@return boolean 启动成功返回true,否则返回false
@usage 
-- UDP配网,需要下载联德盛测试APP,2.0版本
wlan.oneShotStart(0)
@usage 
-- SOCKET配网,需要下载联德盛测试APP,2.0版本
wlan.oneShotStart(1)
@usage 
-- AP网页配网,手机搜索wifi "W600APWEB", 密码12345678. 连上之后,保持wifi连接,浏览器访问 192.168.168.1, 按提示输入.
wlan.oneShotStart(2, "W600APWEB", "12345678")
@usage
-- 监听配网信息
sys.subscribe("WLAN_PW_RE", function(ssid, passwd)
    if ssid then
        log.info("wlan", "Got ssid and passwd", ssid, passwd)
    else
        log.info("wlan", "oneshot fail")
    end
end)
*/
static int l_wlan_oneshot_start(lua_State *L) {
    WM_ONESHOT_MODE mode = (WM_ONESHOT_MODE)luaL_optinteger(L, 1, WM_UDP);
    oneshot_autojoin = luaL_optinteger(L, 2, 1);
    LLOGD("oneshot mode=%lld\n", mode);
    if (mode == WM_APWEB) {
        const char* ssid = luaL_optstring(L, 2, "luatos");
        const char* passwd = luaL_optstring(L, 3, "12345678");
        LLOGD("APWEB ssid=%s passwd=%s", ssid, passwd);
        rt_wlan_set_mode("wlan0", RT_WLAN_STATION);
        rt_wlan_set_mode("wlan1", RT_WLAN_AP);
        rt_wlan_start_ap(ssid, passwd);
    }
    else {
        rt_wlan_set_mode("wlan0", RT_WLAN_STATION);
    }
    int re = wm_oneshot_start(mode, _PW_callback);
    LLOGD("oneshot start re=%ld\n", re);
    lua_pushboolean(L, re == 0);
    return 1;
}

/*
停止配网, 通常不需要调用
@api wlan.oneshotStop()
@return nil 总是没有返回值
@usage 
-- 停止配网
wlan.oneshotStop()
*/
static int l_wlan_oneshot_stop(lua_State *L) {
    wm_oneshot_stop();
    return 0;
}

/*
查询配网状态
@api wlan.oneshotState()
@return boolean 配网中返回true,否则返回false
@usage 
-- 查询
if wlan.oneshotState() then
    log.info("wlan", "配网中")
end
*/
static int l_wlan_oneshot_state(lua_State *L) {
    lua_pushboolean(L, wm_oneshot_get());
    return 1;
}
#endif
//--------------------------

static int l_wlan_join_info(lua_State *L) {
    if (jinfo.ssid[0] != NULL) {
        lua_pushstring(L, jinfo.ssid);
        if (jinfo.passwd[0] != NULL) {
            lua_pushstring(L, jinfo.passwd);
            return 2;
        }
        return 1;
    }
    return 0;
}

/*
获取wifi信号强度值rssi
@function wlan.rssi()
@return int 如果是station模式,返回正的rssi值,否则返回负值
@usage 
-- 信号强度
log.info("wlan", wlan.rssi())
*/
static int l_wlan_rssi(lua_State* L) {
    int rssi;
    wifi_mgmr_rssi_get(&rssi);
    lua_pushinteger(L, rssi);
    return 1;
}

/*
启动airkiss配网线程
@function wlan.airkiss_start()
@return re 启动成功返回1,否则返回0
@usage 
-- 启动airkiss配网
wlan.airkiss_start()
@usage
-- 监听配网信息
sys.subscribe("WLAN_PW_RE", function(ssid, passwd)
    if ssid then
        log.info("wlan", "Got ssid and passwd", ssid, passwd)
    else
        log.info("wlan", "oneshot fail")
    end
end)
*/
// #include "airkiss.h"
// static int l_wlan_airkiss_start(lua_State* L){
//     rt_wlan_set_mode("wlan0", RT_WLAN_STATION);
//     airkiss_set_callback(_PW_callback);
//     lua_pushinteger(L, airkiss_start());
//     return 1;
// }
#define WLAN_NONE    0
#define WLAN_STATION 1
#define WLAN_AP      2

#include "rotable.h"
static const rotable_Reg reg_wlan[] =
{
    { "getMode" ,  l_wlan_get_mode , 0},
    { "setMode" ,  l_wlan_set_mode , 0},
    { "connect" ,     l_wlan_connect , 0},
    { "disconnect",l_wlan_disconnect , 0},
    { "connected" ,l_wlan_connected , 0},
    { "ready" ,    l_wlan_ready , 0},
    { "autoreconnect", l_wlan_autoreconnect, 0},
    { "scan",      l_wlan_scan, 0},
    { "scan_get_info", l_wlan_scan_get_result, 0},
    { "scanResult", l_wlan_scan_get_result, 0},
    { "getMac", l_wlan_get_mac, 0},
    { "get_mac", l_wlan_get_mac, 0},
    { "get_mac_raw", l_wlan_get_mac_raw, 0},
    { "rssi",        l_wlan_rssi, 0},
    //{ "set_mac", l_wlan_set_mac},
    // ---- oneshot
    #ifdef WM_USING_ONESHOT
    { "oneshotStart" ,  l_wlan_oneshot_start , 0},
    { "oneshotStop" ,   l_wlan_oneshot_stop ,  0},
    { "oneshotState" ,  l_wlan_oneshot_state , 0},
    #endif
    { "lastInfo" ,      l_wlan_join_info , 0},
    // { "airkiss_start",  l_wlan_airkiss_start, 0},
    // ---
    
    { "NONE",      NULL , WLAN_NONE},
    { "STATION",   NULL , WLAN_STATION},
    { "AP",        NULL , WLAN_AP},
	{ NULL, NULL , 0}
};

LUAMOD_API int luaopen_wlan( lua_State *L ) {
    reg_wlan_callbacks();
    luat_newlib(L, reg_wlan);
    return 1;
}