import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc, props):
    print("Connected with result code " + str(rc))
    client.subscribe("esp32/check_alive")

def on_message(client, userdata, msg):
    client.publish("esp32/alive_status", 1)
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.tls_set()
client.username_pw_set("esp", "_ledarray_nugget")

client.on_connect = on_connect
client.on_message = on_message

client.connect("5686adbdc3644dca8e63a851e72c3b21.s1.eu.hivemq.cloud", 8883, 60)

client.loop_forever()