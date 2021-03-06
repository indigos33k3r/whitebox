// These structures are known in the server code and
// opaque from the perspective of the radio code.
struct libwebsocket_context;
struct client_context;
struct cJSON;

// This class is known in the radio code and opaque
// from the perspective of the server code.
class radio_context;

/* Barf with a spoon, Chris! */
#define WEBSOCKET_FD 0
#define WHITEBOX_FD 1
#define FILE_SOURCE_FD 2

class WriteBuffer {
private:
  unsigned char * const	storage;
  unsigned char * const	buf;
  WriteBuffer *		next;
  const size_t		maxLength;
  size_t		size;

  WriteBuffer &		operator =(const WriteBuffer &);
  			WriteBuffer(const WriteBuffer &);

  void			too_large();

public:
  inline unsigned char *data() const { return buf; }

  inline size_t		length() const { return size; }

  inline void		length(size_t l) {
			  if ( l > maxLength )
			    too_large();
                          size = l;
			}

  inline void		link(WriteBuffer * that) { next = that; }
  inline WriteBuffer *	link() const { return next; }

  			WriteBuffer(size_t length, uint32_t type);
			~WriteBuffer();
};

class client_info {
public:
  const char *	path;
  const char *	hostname;
  const char *	ip_address;
  const char *	user_agent;
  bool		certificate_is_valid;
  const char *	callsign;
  const char *	name;
  const char *	email;

  void		cleanup();
};

extern void	poll_start_fd(libwebsocket_context *, int fd, int events, int type);
extern void	poll_change_fd(int fd, int mode);
extern void	poll_end_fd(int fd);

extern void		radio_end(radio_context *, const client_info *);
extern void		radio_get_status(radio_context *, const client_info *, cJSON * json);
extern radio_context *	radio_start(client_context *, const client_info *);
extern void		radio_receive(radio_context *, const client_info *);
extern void		radio_set(radio_context *, const client_info *, const cJSON * json);
extern void		radio_transmit(radio_context *, const client_info *);

extern void		radio_data_in(
 radio_context *	radio,
 const client_info *	info,
 uint32_t		type,
 const void *		data,
 size_t			length);

void		server_end(libwebsocket_context *);

void		server_data_out(
                 client_context *	opaque,
                 WriteBuffer *		buffer);

void		server_service_fd(libwebsocket_context *, pollfd * pfd);
libwebsocket_context * server_start(const char * device, int port, bool use_ssl);
