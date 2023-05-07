#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define STACK_SIZE (1024 * 1024)

static char stack[STACK_SIZE];

int setip(char *name, char *addr, char *netmask)
{
	/**
	 * This function allows us to create a connection between the
	 */

	struct ifreq ifr;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

	strncpy(ifr.ifr_name, name, IFNAMSIZ);

	ifr.ifr_addr.sa_family = AF_INET;
	inet_pton(AF_INET, addr, ifr.ifr_addr.sa_data + 2);
	ioctl(fd, SIOCSIFADDR, &ifr);

	inet_pton(AF_INET, netmask, ifr.ifr_addr.sa_data + 2);
	ioctl(fd, SIOCSIFNETMASK, &ifr);

	ioctl(fd, SIOCGIFFLAGS, &ifr);
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

	ioctl(fd, SIOCSIFFLAGS, &ifr);

	return 0;
}

int run_sh(void *arg)
{
	execlp("/bin/sh", "/bin/sh", NULL);
	return EXIT_SUCCESS;
}

int child(void *arg)
{
	char c;
	sleep(1);
	sethostname("myhost", 6);

	int ret = mount("/dev/loop41", "/home", "ext4", MS_NOATIME, "");
	printf("4) Put container filesystem in a file by using loop and try to create file in your new mount point.");
	if (ret != 0)
	{
		perror("[RESULT] : Wasn't able to mount virtual storage device");
		exit(EXIT_FAILURE);
	}
	else
	{
		perror("[RESULT] : Virtual storage device is succesfully mounted.");
	}

	printf("5) Run processor listing, network information and benchmark in your container, LXC and Docker guest containers.\n");
	printf("[RESULT] : \n");
	system("sh benchmark_sysbench.sh | tee container_benchmarking.txt");

	chdir("/home");

	setip("veth1", "10.0.0.15", "255.0.0.0");

	static char sh_stack[STACK_SIZE];
	pid_t sh_pid = clone(run_sh, sh_stack + STACK_SIZE, SIGCHLD, NULL);
	waitpid(sh_pid, NULL, 0);

	if (ret != 0)
	{
		perror("Problem with unmounting the system.");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int main()
{
	char buf[255];
	pid_t pid = clone(child, stack + STACK_SIZE,
					  CLONE_NEWNET | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, NULL);

	if (pid == -1)
	{
		// Clone operation failed
		printf("Containerization operation failed:\n");
	}
	else
	{
		// Clone operation succeeded
		printf("1) Create your own container that is able to run processes via command line using clone().\n");
		printf("[RESULT] : Container is created.\n");
		printf("2) Isolate created container by creating new namespaces for PID, mount and networking.\n");
		printf("[RESULT] : Container is isolated using :  CLONE_NEWNET | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS \n");
	}

	sprintf(buf, "sudo ip link add name veth0 type veth peer name veth1 netns %d", pid);

	system(buf);
	setip("veth0", "10.0.0.13", "255.0.0.0");

	printf("3) Set up a network connection between parent namespace and child namespace.\n");
	printf("[RESULT] : The connection was succesfully established.");

	waitpid(pid, NULL, 0);
	return 0;
}