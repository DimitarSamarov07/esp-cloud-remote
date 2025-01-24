import mqtt from 'mqtt';

const protocol = 'mqtt'
const host = '93.155.224.232'
const port = '5728'

const connectURL = `${protocol}://${host}:${port}`

// const responseTopic = 'ac/report'
const controlTopic = 'ac/control'
const wifiConnectionTopic = 'connection/wifi'

class ESPClient {
    constructor(options) {
        this.client = mqtt.connect(connectURL, options)
        this.clientId = options.clientId
        this.subscribedTopics = new Set();

        this.client.on('connect', () => {
            console.log(`[${this.clientId}] Connected`)
        })

        this.client.on('error', () => {
            console.log(`[${this.clientId}] Couldn't connect because of an error`)
        })

        this.client.on('message', (topic, message) => {
            if(this.subscribedTopics.has(topic)) {
                console.log(`[${this.clientId}] Message received on topic '${topic}': ${message}`)
            }
        })

        this.client.on('disconnect', () => {
            console.log(`[${this.clientId}] Disconnected`)
        })
    }

    publish(topic, message) {
        this.client.publish(topic, message, (error) => {
            if (error) {
                console.log(`[${this.clientId}] Couldn't publish`)
            } else {
                console.log(`[${this.clientId}] Published to ${topic}`)
            }
        })

    }

    subscribe(topic) {
        this.client.subscribe(topic, (error) => {
            if (error) {
                console.error(`[${this.clientId}] Failed to subscribe to topic`, error);
            } else {
                console.log(`[${this.clientId}] Subscribed to ${topic}`)
                this.subscribedTopics.add(topic)
            }
        })
    }

    turnLEDOn() {
        this.client.publish(controlTopic, 'TURN_LED_ON')
        console.log(`[${this.clientId}] Sent TURN_LED_ON command`)
    }

    turnLEDOff() {
        this.client.publish(controlTopic, 'TURN_LED_OFF')
        console.log(`[${this.clientId}] Sent TURN_LED_OFF command`)
    }

    changeWifi(ssid, pass) {
        this.client.publish(wifiConnectionTopic, `${ssid}/${pass}`)
        console.log(`[${this.clientId}] Sent updated WiFi config`)
    }
}

const esp = new ESPClient({
    clientId: 'mqtt_pc1', // we can use UUID
    clean: false,
    connectTimeout: 4000,
    reconnectPeriod: 1000,
})

esp.turnLEDOff()
esp.changeWifi('OPTELA', '')