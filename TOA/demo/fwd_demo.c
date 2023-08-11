#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define LISTEN_PORT 4321

int Listen(uint16_t port)
{
	int skfd;
	int ret;
	struct sockaddr_in addr;

	ret = socket(AF_INET, SOCK_STREAM, 0);
	if (ret < 0) {
		perror("socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(0);
	ret = bind(skfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0) {
		perror("bind");
		goto err_out;
	}

	return skfd;
err_out:
	close(skfd);
	return -1;
}

void do_service(int fd)
{
	struct sockaddr_in peeraddr;
	socklen_t addrlen;

	// get sock peer name
	addrlen = sizeof(peeraddr);
	if (getpeername(fd, (struct sockaddr*)&peeraddr, &addrlen) < 0) {
		perror("getpeername");
		return;
	}


}

int main(int argc, char** argv)
{
	int liskfd;

	liskfd = Listen(LISTEN_PORT);
	if (liskfd < 0) {
		return -1;
	}

	for (;;) {
		int confd;
		confd = accept(liskfd, NULL, NULL);
		if (confd > 0) {
			do_service(confd);
			close(confd);
		}
	}

	return 0;
}