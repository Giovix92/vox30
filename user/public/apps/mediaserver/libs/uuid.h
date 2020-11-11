#ifndef _UUID_H
#define _UUID_H

  typedef struct _uuid_upnp {
      unsigned32          time_low;
      unsigned16          time_mid;
      unsigned16          time_hi_and_version;
      unsigned8           clock_seq_hi_and_reserved;
      unsigned8           clock_seq_low;
      byte                node[6];
  } uuid_upnp;

  /* uuid_create -- generate a UUID */
  int uuid_create(uuid_upnp * id);
  int uuid_create2( uuid_upnp * uid, char *mac ); // add by john
  void uuid_unpack(uuid_upnp *u, char *out);	// out will be xxxx-xx-xx-xx-xxxxxx format

  /* uuid_create_from_name -- create a UUID using a "name"
     from a "name space" */
  void uuid_create_from_name(
    uuid_upnp * uid,        /* resulting UUID */
    uuid_upnp nsid,          /* UUID to serve as context, so identical
                             names from different name spaces generate
                             different UUIDs */
    void * name,          /* the name from which to generate a UUID */
    int namelen           /* the length of the name */
  );

  /* uuid_compare --  Compare two UUID's "lexically" and return
          -1   u1 is lexically before u2
           0   u1 is equal to u2
           1   u1 is lexically after u2
     Note:   lexical ordering is not temporal ordering!
  */


  #endif
  
  int uuid_compare(uuid_upnp *u1, uuid_upnp *u2);
