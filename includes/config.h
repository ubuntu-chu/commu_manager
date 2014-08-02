#ifndef    _CONFIG_H
#define    _CONFIG_H

#include <io_node.h>
#include <device_node.h>
#include <process_node.h>
#include <protocol_node.h>
#include <power_node.h>

//工程配置定义
class project_config{
public:
	project_config(){}
	~project_config(){}

	void describe_set(const char *describe){strncpy(describe_, describe, sizeof(describe_));}
	const char *describe_get(void){return describe_;}

	bool run_led_on(void)
	{
        led_node *pnode         = led_.led_node_get(def_LED_RUN_STRING);
        if (NULL != pnode){
            return pnode->led_on();
        }

        return false;
	}
	bool run_led_off(void)
	{
        led_node *pnode         = led_.led_node_get(def_LED_RUN_STRING);
        if (NULL != pnode){
            return pnode->led_off();
        }

        return false;
	}

	bool alarm_led_on(void)
	{
        led_node *pnode         = led_.led_node_get(def_LED_ALARM_STRING);
        if (NULL != pnode){
            return pnode->led_on();
        }

        return false;
	}
	bool alarm_led_off(void)
	{
        led_node *pnode         = led_.led_node_get(def_LED_ALARM_STRING);
        if (NULL != pnode){
            return pnode->led_off();
        }

        return false;
	}

	led_config &led_config_get(void){return led_;}

	void power_config_set(power_config &config){power_ 	= config;}
	power_config &power_config_get(void){return power_;}

	void process_config_set(process_config &config){process_ 	= config;}
	process_config &process_config_get(void){return process_;}

	void protocol_config_set(protocol_config &config){protocol_ 	= config;}
	protocol_config &protocol_config_get(void){return protocol_;}

	void io_config_set(io_config &config){io_ 	= config;}
	io_config &io_config_get(void){return io_;}

	void device_config_set(device_config &config){device_ 	= config;}
	device_config &device_config_get(void){return device_;}

	void log_lev_set(char lev)
	{
	    log_level_              = lev;
	}
	int log_lev_get(void)
	{
	    return log_level_;
	}

private:
	char   			            describe_[def_DESCRIBE_MAX_LEN];
	char                       log_level_;
	led_config 				    led_;
	power_config 				power_;
	process_config 				process_;
	protocol_config 			protocol_;
	io_config 					io_;
	device_config 				device_;
};

/******************************************************************************
 *                             END  OF  FILE
******************************************************************************/
#endif
