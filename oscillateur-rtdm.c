#include <linux/version.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <rtdm/rtdm_driver.h>
#include <native/task.h>
#include <native/pipe.h>


// 138 Broche 10 du port J3 (Expansion A) de la Pandaboard
#define GPIO_OSCILLATEUR 138


// La période de l'oscillateur
static int periode_us = 1000;
module_param(periode_us, int, 0644); // Macro utilisee pour dire que l'on est succeptible de rentrer un parametre

// Le timer
static void timer_oscillateur(rtdm_timer_t *);
static rtdm_timer_t rtimer;

static RT_PIPE pipe;
static void task(void *arg);
static void changerPeriode(int nouvellePer);

// Pour afficher des messages formates dans le noyau
static void debug(char* mes);
static void debugInt(int mes);
static void afficherErreur(char* mes);

static int __init init_oscillateur(void)
{
	rtdm_task_t task_desc;
	int err;

	if ((err = gpio_request(GPIO_OSCILLATEUR, THIS_MODULE->name)) != 0) {
		afficherErreur("GPIO gpio_request: %d", err);
		return err;
	}

	if ((err = gpio_direction_output(GPIO_OSCILLATEUR, 1)) != 0) {
		gpio_free(GPIO_OSCILLATEUR);
		afficherErreur("GPIO gpio_direction_output: %d", err);
		return err;
	}

	if ((err = rtdm_timer_init(&rtimer, timer_oscillateur, "Oscillateur")) != 0) {
		gpio_free(GPIO_OSCILLATEUR);
		afficherErreur("rtdm_timer_init: %d", err);
		return err;
	}

	if ((err = rtdm_timer_start(&rtimer, periode_us * 1000, periode_us * 1000, RTDM_TIMERMODE_RELATIVE)) != 0) {
		rtdm_timer_destroy(&rtimer);
		gpio_free(GPIO_OSCILLATEUR);
		afficherErreur("rtdm_timer_start: %d", err);
		return err;
	}

	if ((err = rtdm_task_init(&task_desc, "recuperation periode", &task, NULL, 99)) != 0)
	{
		rtdm_timer_destroy(&rtimer);
		gpio_free(GPIO_OSCILLATEUR);
		afficherErreur("Erreur du lancement de la tache %d", err);
		return err;
	}	
	
	//Initialisation du pipe
	if ((err = rt_pipe_create(&pipe, "rtp0", P_MINOR_AUTO, 0)) != 0){
		rtdm_timer_destroy(&rtimer);
		gpio_free(GPIO_OSCILLATEUR);
		afficherErreur("probl�me cr�ation pipe %d", err);
		return err;
	}

	return 0;
}



static void __exit exit_oscillateur(void)
{
	rt_pipe_delete(&pipe);
	rtdm_timer_stop(&rtimer);
	rtdm_timer_destroy(&rtimer);
	gpio_free(GPIO_OSCILLATEUR); // on libere le gpio, sinon la prochaine fois il dira que le gpio est utilise
}



static void timer_oscillateur(rtdm_timer_t * unused)
{
	static int value = 0;
	gpio_set_value(GPIO_OSCILLATEUR, value);
	value = 1 -	value;
}

static void task(void *arg)
{
	int nouvellePer;

	while (1){		
		if (rt_pipe_read(&pipe, &nouvellePer, sizeof(int), TM_NONBLOCK) != 0)
			changerPeriode(nouvellePer)		
		
		rtdm_task_wait_period();
	}	
}

static void changerPeriode(int periode){	
	switch (periode) {
		case 1:
			nouvellePer = 50;
			break;
		case 2:
			nouvellePer = 40;
			break;
		case 3:
			nouvellePer = 30;
			break;
		case 4:
			nouvellePer = 20;
			break;
		case 5:
			nouvellePer = 10;
			break;
		default:
			nouvellePer = 50;
	}

	//Arreter et relancer la clock
	rtdm_timer_stop(&rtimer);
	rtdm_timer_start(&rtimer, nouvellePer*1000, nouvellePer*1000, RTDM_TIMERMODE_RELATIVE);	
}

static void debug(char* mes){
	rtdm_printk(KERN_INFO "[DEBUG]: %s\n", mes);
}

static void debugInt(int mes){
	rtdm_printk(KERN_INFO "[DEBUG]: %d\n", mes);
}

static void afficherErreur(char* mes){
	rtdm_printk(KERN_INFO "[ERREUR]: %s\n", mes);
}

module_init(init_oscillateur);
module_exit(exit_oscillateur);
MODULE_LICENSE("GPL");
