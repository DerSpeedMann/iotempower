// device.cpp
// author: ulno
// created: 2018-07-16

#include <Arduino.h>
#include "device.h"

void Subdevice::init_log() {
    ulog(F("subdevice init: subname: >%s<"), name.as_cstr());
}

void Subdevice::init(const char* subname, bool subscribed) {
    name.from(subname);
    _subscribed = subscribed;
    init_log();
}

void Subdevice::init(const __FlashStringHelper* subname, bool subscribed) {
    name.from(subname);
    _subscribed = subscribed;
    init_log();
}

bool Subdevice::call_receive_cb(Ustring& payload) {
    ulog(F("Calling receive callback >%s< >%s<"), 
        name.as_cstr(), payload.as_cstr());
    if(receive_cb != NULL) {
        return receive_cb(payload);
    }
    ulog(F("!failure receive_cb!"));
    return false;
}

//#include <device-manager.h>
// Just define add_device from here - to avoid messed up dependencies
bool add_device(Device& device);

Device::Device(const char* _name, unsigned long __pollrate_us) {
    name.from(_name);
    pollrate_us(__pollrate_us);
    ulog(F("Device: Adding device: %s pollrate: %lu"), name.as_cstr(), _pollrate_us);
    if(!add_device(*this)) {
        ulog(F("Device %s couldn't be added - max devices reached."), _name);
    }
}

#ifdef mqtt_discovery_prefix
void Device::create_discovery_info(const String& type,
        bool state_topic, const char* state_on, const char* state_off,
        bool command_topic, const char* payload_on, const char* payload_off,
        const String& extra_json) { // TODO: for inverted on/off this doesn't really work -> rethink
    
// should work dynamically  
//    discovery_config_topic.reserve(128); // extract lengths into constants
//    discovery_info.reserve(512);

    String slash(F("/"));
    String qmarks(F("\""));
    String comma_marks(F(", \""));
    String name_s(name.as_cstr());
    String topic_s = String(mqtt_topic) + slash + name_s;
    String hostname_s(HOSTNAME);

    discovery_config_topic = String(F(mqtt_discovery_prefix)) + slash + type + slash
        + hostname_s + slash + name_s + String(F("/config"));

    discovery_info = String(F("{ \"name\": \"")) + hostname_s + String(F(" ")) + String(name.as_cstr()) + qmarks;
    if(state_topic) {
        discovery_info += comma_marks + String(F("state_topic\": \"")) + topic_s + qmarks;
    }
    if(state_on)
        discovery_info += comma_marks + String(F("state_on\": \"")) + String(state_on) + qmarks;
    if(state_off)
        discovery_info += comma_marks + String(F("state_off\": \"")) + String(state_off) + qmarks;
    if(command_topic) {
        discovery_info += comma_marks + String(F("command_topic\": \"")) 
                            + topic_s + String(F("/set")) + qmarks;
    }
    if(payload_on)
        discovery_info += comma_marks + String(F("payload_on\": \"")) + String(payload_on) + qmarks;
    if(payload_off)
        discovery_info += comma_marks + String(F("payload_off\": \"")) + String(payload_off) + qmarks;
    if(extra_json.length() > 0) {
        discovery_info += F(", ");
        discovery_info += extra_json;
    }
    discovery_info += F(" }");
    
    // Serial.print(F("discovery topic: "));
    // Serial.println(discovery_config_topic);
    // Serial.print(F("discovery info: "));
    // Serial.println(discovery_info);
}
#endif

////AsyncMqttClient disabled in favor of PubSubClient
//bool Device::publish(AsyncMqttClient& mqtt_client, Ustring& node_topic) {
bool Device::publish(PubSubClient& mqtt_client, Ustring& node_topic) {
    bool published = false;
    Ustring topic;
    bool first = true;
    subdevices_for_each( [&] (Subdevice& sd) {
        const Ustring& val = sd.value();
        if(!val.empty()) {
            // construct full topic
            topic.copy(node_topic);
            topic.add(F("/"));
            topic.add(get_name());
            const Ustring& sd_name = sd.get_name();
            if(!sd_name.empty()) { // only add, if name given
                topic.add(F("/"));
                topic.add(sd.get_name());
            }
            if(first) {
                Serial.print(F(" "));
                first = false;
            }
            else Serial.print(F("|"));
            Serial.print(topic.as_cstr()+node_topic.length()+1);
            Serial.print(F(":"));
            Serial.print(val.as_cstr());

            ////AsyncMqttClient disabled in favor of PubSubClient
            // if(!mqtt_client.publish(topic.as_cstr(), 0, false, val.as_cstr())) {
            if(!mqtt_client.publish(topic.as_cstr(), (uint8_t*) val.as_cstr(), (unsigned int)val.length(), false)) {
                Serial.print(F("!publish error!"));
                // TODO: signal error and trigger reconnect - necessary?
                return false;
            } else {
                delay(1); // give room for publish to be sent out TODO: implement sync send
            }
            published = true;
        }
        return true; // continue loop
    } ); // end for_each - iterate over subdevices
    if(published) _needs_publishing = false;
    return published;
}

#ifdef mqtt_discovery_prefix
////AsyncMqttClient disabled in favor of PubSubClient
//bool Device::publish_discovery_info(AsyncMqttClient& mqtt_client) {
bool Device::publish_discovery_info(PubSubClient& mqtt_client) {
    if(discovery_config_topic.length()>0) { // only if exists
        ulog(F("Publishing discovery info for %s."), name.as_cstr());
        // TODO: check if retained and/or some cleanup necessary
        ////AsyncMqttClient disabled in favor of PubSubClient
        // if(!mqtt_client.publish(discovery_config_topic.c_str(), 0, true, discovery_info.c_str(), discovery_info.length())) {
        //     ulog(F("!discovery publish error!"));
        //     // TODO: signal error and trigger reconnect - necessary?
        //     return false;
        // } else {
        //     // make sure it gets sent
        //     delay(10);  // This delay is important to prevent overflow of network buffer TODO: implement sync publish mechanism
        // }
        unsigned int left = discovery_info.length();
        if(!mqtt_client.beginPublish(discovery_config_topic.c_str(), left, true)) {
            ulog(F("!discovery publish init error!"));
        }
        const uint8_t* start = (uint8_t*) discovery_info.c_str();
        while(true) {
            if(left>128) {
                if(!mqtt_client.write(start, 128)) break;
                start += 128;
                left -= 128;
            } else {
                if(!mqtt_client.write(start, left)) break;
                left = 0;
                break;
            }
        }
        if(left>0) {
            ulog(F("!discovery publish write error!"));
        }
        if(!mqtt_client.endPublish()) {
            ulog(F("!discovery publish end error!"));
            return false;
        }
    }
    return true;
}
#endif



bool Device::poll_measure() {
    if(started()) { // only if device active
        if( micros() - last_poll >= _pollrate_us) { // only measure so often
            last_poll = micros();
            measure_init(); // something might have to be executed here to init each time, for example i2c setup
            if(!measure()) {  // measure new value or trigger physical update
//                ulog("poll measure no val: %s, %lu", name.as_cstr(), _pollrate_us); // TODO: remove/debug
                // no new value generated
                yield(); // measure might have taken long, give some time to system TODO: do we have to take care of mqtt here too?
                // re-use last value(s), if measurement not successful
                subdevices.for_each( [](Subdevice& sd) {
                    sd.measured_value.copy(sd.current_value);
                    return true; // continue loop
                } );
            } else { // new value generated
//                ulog("poll measure new val: %s, %lu", name.as_cstr(), _pollrate_us); // TODO: remove/debug
                yield(); // measure might have taken long, give some time to system TODO: do we have to take care of mqtt here too?
                //ulog("poll rate measure new val: %lu", _pollrate_us); // TODO: remove/debug
                if(_ignore_case) { // if necessary, lower them all
                    subdevices.for_each( [](Subdevice& sd) {
                        sd.measured_value.lower();
                        return true; // continue loop
                    } );
                }
            }
            measure_exit(); // something might have to be executed here to exit each time, for example i2c setup
            
            // a current value is now in measured_value(s)
            // check if it needs to be filtered
            if(_filter_cb != NULL && ! _filter_cb()) {
                // if filter defined but indicates to ignore the current measurement
                return false; // end here with no update
            }
            // The measured value(s) is/are now an actual, valid new measurement
            return true;
        }
    }
    return false;
}

bool Device::check_changes() {
    // check if the value has changed/is updated and call on_change_cb
    bool updated = false;
    bool changed = false;
    
    subdevices.for_each( [&] (Subdevice& sd) {
        if(!sd.measured_value.equals(sd.current_value)) {
            sd.current_value.copy(sd.measured_value);
            changed = true;
            if(_report_change) {
                updated = true;
                _needs_publishing = true;
                Serial.print(F("Needs publishing: "));
                Serial.print(name.as_cstr());
                if(!sd.get_name().empty()) {
                    Serial.print(F("/"));
                    Serial.print(sd.get_name().as_cstr());
                }
                Serial.print(F(":"));
                Serial.println(sd.measured_value.as_cstr());
            }
        }
        return true; // continue loop
    } ); // end for each subdevices

    if(changed) {
        call_on_change_callback();
    }
    return updated;
}

static Ustring value_error;

// TODO: why can't I make this const/const?
Ustring& Device::value(unsigned long index) {
    Subdevice* sd = subdevice(index);
    if(sd == NULL) {
        ulog(F("Error in value(). Device: %s"), name.as_cstr());
        value_error.from(F("subdevice value error"));
        return value_error;
        // TODO: should this crash here?
    }
    return sd->value();
}

Ustring& Device::measured_value(unsigned long index) {
    Subdevice* sd = subdevice(index);
    if(sd == NULL) {
        ulog(F("Error in measured_value(): Device: %s"), name.as_cstr());
        value_error.from(F("no subdevice"));
        return value_error;
    }
    return sd->measured_value;
}
