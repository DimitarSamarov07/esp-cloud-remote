#ifndef MQTT_TOPICS_H
#define MQTT_TOPICS_H

#define MQTT_TOPIC_AC_CONTROL       "ac/control/%s"
#define MQTT_TOPIC_WIFI_CONFIG      "connection/wifi/%s"
#define MQTT_TOPIC_AC_STATUS        "ac/status/%s"
#define MQTT_TOPIC_LWT              "ac/status/%s"  // Last Will Testament

#define MQTT_TOPIC_COUNT 2

#define MQTT_QOS_CONTROL    1
#define MQTT_QOS_STATUS     1

#endif