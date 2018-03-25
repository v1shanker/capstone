/** @file motor_controller_task.h
 *
 *  @brief Constants and API for motor controller task
 *
 *  @author Vikram Shanker (vshanker@andrew.cmu.edu)
 *
 *  @bug No known bugs.
*/

#ifndef _MOTOR_CONTROLLER_TASK_H_

#define MOTOR_CONTROLLER_STACK_DEPTH (2048)
#define MOTOR_CONTROLLER_TASK_PRIO   (2)

/** @brief spins up task the interfaces with the
 *   Hercules motor controller board.
 *
 *  @param void
 *
 *  @return void
 */
void create_motor_controller_task( void );

#endif /* _MOTOR_CONTROLLER_TASK_H_ */
