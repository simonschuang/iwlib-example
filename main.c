#include <stdio.h>
#include <unistd.h>
#include <iwlib.h>

void do_scan(int sock, iwrange *range)
{
	wireless_scan_head head;
	wireless_scan *result;
	char buffer[128];

	/* Perform the scan */
	if (iw_scan(sock, "wlan0", range->we_version_compiled, &head) < 0) {
		printf("Error during iw_scan. Aborting.\n");
		return;
	}

	/* Traverse the results */
	result = head.result;
	while (NULL != result) {
		printf("ESSID: %s\n", result->b.essid);
		//if ((result->b.key_flags & IW_ENCODE_DISABLED) == IW_ENCODE_DISABLED)
		if (result->b.has_key) {
			/* Display the key */
			iw_print_key(buffer, sizeof(buffer), result->b.key, result->b.key_size,
				     result->b.key_flags);
			printf("key: %s\n", buffer);
		}
		if (result->has_ap_addr) {
			printf("sawap: %s\n", iw_sawap_ntop(&result->ap_addr, buffer));
		}
		if (result->b.has_freq) {
			iw_print_freq_value(buffer, sizeof(buffer), result->b.freq);
			printf("freq: %s\n", buffer);
		}
		if (result->has_maxbitrate) {
			iw_print_bitrate(buffer, sizeof(buffer), result->maxbitrate.value);
			printf("bitrate: %s\n", buffer);
		}
		if (result->has_stats) {
			iw_print_stats(buffer, sizeof(buffer), &result->stats.qual, range, 1);
			printf("stats: %s\n", buffer);
		}
		result = result->next;
	}

	result = head.result;
	/* free results */
	while (result && result->next) {
		wireless_scan *tmp;
		tmp = result->next;
		free (result);
		result = tmp;
	}
	if (result) {
		free(result);
	}
}

int main(void)
{
	wireless_scan *result;
	iwrange range;
	char buffer[128];
	int sock, i;

	sock = iw_sockets_open();

	/* Get some metadata to use for scanning */
	if (iw_get_range_info(sock, "wlan0", &range) < 0) {
		printf("Error during iw_get_range_info. Aborting.\n");
		return -1;
	}

	for (i=0; i<5; i++) {
		printf("\033[2J");
		do_scan(sock, &range);
		sleep(1);
	}

	iw_sockets_close(sock);
	return 0;
}
