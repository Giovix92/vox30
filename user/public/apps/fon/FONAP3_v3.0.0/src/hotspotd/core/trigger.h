#ifndef TRIGGER_H_
#define TRIGGER_H_

/**
 * Prototype of a trigger callback
 * See the desciption of the fields si_code and si_status in the manpage for
 * sigaction (man 2 sigaction) for a description of code and status parameters.
 */
typedef void (trigger_cb)(void *context, short code, short status);

struct trigger_handle;

/**
 * Run a trigger and register a callback.
 * Use the trigger command cmd (may include parameters) and append additional
 * parameters args to it. Then run the executable with the environment env.
 * When the external process terminates, run callback cb with parameter ctx.
 *
 * Returns a trigger handle on success.
 */
struct trigger_handle* trigger_run(const char *cmd, char *const *args,
		char *const *env, trigger_cb *cb, void *ctx);

/**
 * Cancel the execution of a given trigger.
 * Cancel the execution of a given trigger by sending SIGTERM to the process
 * and prevent the trigger callback to be run when the process terminates.
 */
void trigger_cancel(struct trigger_handle *handle);


/**
 * Wait for the execution of a given trigger to finish.
 */
void trigger_wait(struct trigger_handle *handle);

#endif /* TRIGGER_H_ */
