from Adafruit_IO import MQTTClient
import time
import sys

AIO_USERNAME = "USERNAME" 
AIO_KEY = "API KEY" 
AIO_FEED = "pc-status"

HEARTBEAT_MESSAGE = 1
HEARTBEAT_INTERVAL = 30


def on_connect(client):
    print("Connected successfully to Adafruit IO")

def on_disconnect(client):
    print("Disconnected from Adafruit IO")

if __name__ == "__main__":
    client = MQTTClient(AIO_USERNAME, AIO_KEY)

    client.on_connect = on_connect
    client.on_disconnect = on_disconnect

    try:
        print(f"Connecting to Adafruit IO")
        client.connect()
    except Exception as e:
        print(f"Could not connect to MQTT broker: {e}")
        sys.exit()

    client.loop_background()

    try:
        while True:
            if not client.is_connected():
                print("Connection lost. Attempting to reconnect")
                try:
                    client.connect()
                    print("Reconnected successfully")
                except Exception as e:
                    print(f"Failed to reconnect: {e}")
                    print("Retrying in 15 seconds")
                    time.sleep(15)
                    continue
            if(HEARTBEAT_MESSAGE > 4): HEARTBEAT_MESSAGE = 1
            else: HEARTBEAT_MESSAGE = HEARTBEAT_MESSAGE + 1
            print(f"Sending heartbeat '{HEARTBEAT_MESSAGE}' to feed '{AIO_FEED}'")
            client.publish(AIO_FEED, HEARTBEAT_MESSAGE)
            
            time.sleep(HEARTBEAT_INTERVAL)

    except KeyboardInterrupt:
        print("\nStopping")
    finally:
        client.publish(AIO_FEED, "0")
        time.sleep(1)
        
        client.disconnect()
        print("Disconnected")

