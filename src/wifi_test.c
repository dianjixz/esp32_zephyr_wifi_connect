/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <errno.h>
#include <zephyr/net/wifi_mgmt.h>

typedef enum
{
    WIFI_NONE = 0,
    WIFI_CONNECT_ING,
    WIFI_CONNECT
} wifi_status_t;
struct wifi_bus_data_t
{
    wifi_status_t status;
    struct k_timer wifi_timer;
} wifi_bus_data;

struct wifi_connect_req_params params;

// void wifi_timer_expiry_function(struct k_timer *timer_id)
// {
//     struct net_if *iface = net_if_get_default();
//     net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface,
//                 &params, sizeof(struct wifi_connect_req_params));

//     if (wifi_bus_data.status == WIFI_NONE)
//     {

//         net_mgmt(NET_REQUEST_WIFI_CONNECT, iface,
//                  &params, sizeof(struct wifi_connect_req_params));
//     }
//     else if(wifi_bus_data.status == WIFI_CONNECT)
//     {
//         k_timer_stop(&wifi_bus_data.wifi_timer);
//     }
// }

static uint32_t scan_result = 1;
static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint32_t mgmt_event, struct net_if *iface)
{
    switch (mgmt_event)
    {
    case NET_EVENT_WIFI_SCAN_RESULT:
    {
        // CB中的信息就是scan的结果
        const struct wifi_scan_result *entry =
            (const struct wifi_scan_result *)cb->info;
        uint8_t mac_string_buf[sizeof("xx:xx:xx:xx:xx:xx")];

        scan_result++;

        if (scan_result == 1U)
        {
            printk("\n%-4s | %-32s %-5s | %-13s | %-4s | %-15s | %s\n",
                   "Num", "SSID", "(len)", "Chan (Band)", "RSSI", "Security", "BSSID");
        }

        printk("%-4d | %-32s %-5u | %-4u (%-6s) | %-4d | %-15s | %s\n",
               scan_result, entry->ssid, entry->ssid_length, entry->channel,
               wifi_band_txt(entry->band),
               entry->rssi,
               wifi_security_txt(entry->security),
               ((entry->mac_length) ? net_sprint_ll_addr_buf(entry->mac, WIFI_MAC_ADDR_LEN, mac_string_buf,
                                                             sizeof(mac_string_buf))
                                    : ""));
    }
    break;
    case NET_EVENT_WIFI_SCAN_DONE:
    {
        // CB中的信息是wifi状态
        const struct wifi_status *status = (const struct wifi_status *)cb->info;
        if (status->status)
        {
            printk("Scan request failed (%d)\n", status->status);
        }
        else
        {
            printk("Scan request done\n");
        }
    }
    break;
    case NET_EVENT_WIFI_CONNECT_RESULT:
    {
        // CB中的信息是wifi状态
        const struct wifi_status *status = (const struct wifi_status *)cb->info;
        if (status->status)
        {
            printk("Connection request failed (%d)\n", status->status);
        }
        else
        {
            wifi_bus_data.status = WIFI_CONNECT;
            printk("Connected\n");
        }
    }
    break;
    case NET_EVENT_WIFI_DISCONNECT_RESULT:
    {
        // CB中的信息是wifi状态
        const struct wifi_status *status = (const struct wifi_status *)cb->info;
        wifi_bus_data.status = WIFI_NONE;
        printk("wifi disconnect !\n");
        // k_timer_start(&wifi_bus_data.wifi_timer, K_SECONDS(1), K_SECONDS(5);
    }
    break;
    case NET_EVENT_WIFI_TWT:
    {
        const struct wifi_twt_params *resp =
            (const struct wifi_twt_params *)cb->info;

        if (resp->resp_status == WIFI_TWT_RESP_RECEIVED)
        {
            printk("TWT response: %s for dialog: %d and flow: %d\n",
                   wifi_twt_setup_cmd2str[resp->setup_cmd], resp->dialog_token, resp->flow_id);

            /* If accepted, then no need to print TWT params */
            if (resp->setup_cmd != WIFI_TWT_SETUP_CMD_ACCEPT)
            {
                printk("TWT parameters: trigger: %s wake_interval_ms: %d, interval_ms: %d\n",
                       resp->setup.trigger ? "trigger" : "no_trigger",
                       resp->setup.twt_wake_interval_ms,
                       resp->setup.twt_interval_ms);
            }
        }
        else
        {
            printk("TWT response timed out\n");
        }
    }
    break;
    default:
        break;
    }
}
static struct net_mgmt_event_callback wifi_mgmt_cb;

void main(void)
{
    wifi_bus_data.status = WIFI_NONE;
    // 初始化，可以指定要响应那些事件
    net_mgmt_init_event_callback(&wifi_mgmt_cb,
                                 wifi_mgmt_event_handler,
                                 (NET_EVENT_WIFI_SCAN_RESULT |
                                  NET_EVENT_WIFI_SCAN_DONE |
                                  NET_EVENT_WIFI_CONNECT_RESULT |
                                  NET_EVENT_WIFI_DISCONNECT_RESULT));
    net_mgmt_add_event_callback(&wifi_mgmt_cb);
    // 配置SSID
    params.ssid = "9";
    params.ssid_length = strlen(params.ssid);
    // 配置channel
    params.channel = WIFI_CHANNEL_ANY;
    // 配置连接密码
    params.psk = "1234567890";
    params.psk_length = strlen(params.psk);
    // 密码验证方式
    params.security = WIFI_SECURITY_TYPE_PSK;
    // 管理帧保护
    params.mfp = WIFI_MFP_OPTIONAL;

    // k_timer_init(&wifi_bus_data.wifi_timer, wifi_timer_expiry_function, NULL);
    // k_timer_start(&wifi_bus_data.wifi_timer, K_SECONDS(1), K_SECONDS(5);

    struct net_if *iface = net_if_get_default();
    net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0);
    k_sleep(K_SECONDS(5));
    // int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface,
    //                     &params, sizeof(struct wifi_connect_req_params));
    // 执行连接
    while (!wifi_bus_data.status)
    {
        int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface,
                           &params, sizeof(struct wifi_connect_req_params));
        printk("will be connect 9\n");
        k_sleep(K_SECONDS(5));
    }

    // printk("will be connect 9\n");

    // // int ret = net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0);
}
