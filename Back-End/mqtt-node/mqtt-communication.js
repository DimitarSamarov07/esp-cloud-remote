import mqtt from 'mqtt';

const protocol = 'mqtt'
const host = '93.155.224.232'
const port = '5728'

const connectURL = `${protocol}://${host}:${port}`

const responseTopic = 'ac/report'
const controlTopic = 'ac/control'
const wifiConnectionTopic = 'connection/wifi'

class ESPClient {
    constructor(options) {
        this.client = mqtt.connect(connectURL, options)
        this.clientId = options.clientId

        this.client.on('connect', () => {
            console.log(`${this.clientId} connected`)
        })

        this.client.on('error', () => {
            console.log(`${this.clientId} couldn't connect because of an error`)
        })

        this.client.on('disconnect', () => {
            console.log(`${this.clientId} disconnected`)
        })
    }

    publish(topic, message) {
        this.client.publish(topic, message)
        console.log(`${this.clientId} published to ${topic}`)
    }

    subscribe(topic) {
        this.client.subscribe(topic)
        console.log(`${this.clientId} subscribed to ${topic}`)
    }

    turnLEDOn() {
        this.client.publish(controlTopic, 'TURN_LED_ON') // [${this.clientId}]
        this.client.publish(responseTopic, `TURN_LED_ON command sent by ${this.clientId}`)
    }

    turnLEDOff() {
        this.client.publish(controlTopic, 'TURN_LED_OFF')
        this.client.publish(responseTopic, `TURN_LED_OFF command sent by ${this.clientId}`)
    }

    changeWifi(ssid, pass) {
        // insecure
        this.client.publish(wifiConnectionTopic, `${ssid}/${pass}`)
        this.client.subscribe()
        console.log(`${this.clientId} sent new wifi data to connection/wifi`)
    }
}

const esp = new ESPClient({
    clientId: 'mqtt_esp1', // we can use UUID
    clean: false,
    connectTimeout: 4000,
    reconnectPeriod: 1000,
})

esp.turnLEDOn();
esp.changeWifi('Hamza A25', 'a44db555')
esp.publish(responseTopic, `\n${esp.clientId} has thanked the bus driver`)

