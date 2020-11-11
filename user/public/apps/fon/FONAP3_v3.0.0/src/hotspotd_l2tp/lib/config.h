#ifndef CONFIG_H_
#define CONFIG_H_

#include <netinet/in.h>
#include <stdbool.h>

#ifdef HOTSPOTD_UBUS
#ifndef __SC_BUILD__
#include <uci_blob.h>
int config_dump_section(struct blob_buf *b, const char *sec, const struct uci_blob_param_list param_list, bool src);
#endif
#endif

int config_init(const char *name);
void config_deinit();
const char* config_get_string(const char *sec, const char *opt, const char *def);
int config_get_int(const char *sec, const char *opt, int def);
int config_get_ipv4(struct in_addr *addr, const char *sec, const char *opt);
int config_get_ipv6(struct in6_addr *addr, const char *sec, const char *opt);
bool config_get_bool(const char *sec, const char *opt, bool def);
int config_add_string(const char *sec, const char *opt, const char *val);
int config_set_string(const char *sec, const char *opt, const char *val);
int config_set_bool(const char *sec, const char *opt, bool val);
int config_foreach_list(const char *sec, const char *opt, void(*cb)(const char*, void*), void *ctx);
int config_foreach_section(const char *type, void(*cb)(const char *, void*), void *ctx);
int config_section_exists(const char *name);
int config_commit();
#endif /* CONFIG_H_ */
