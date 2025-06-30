first = "caabbbbaddb"

def remove_run_rec(string, idx, cnt, head):
	if (idx == len(string)):
		return string if cnt < 3 else string[:len(string) - cnt]
	
	first = string[idx]
	if (first == head):
		return remove_run_rec(string, idx + 1, cnt + 1, first)
	
	if (cnt >= 3):
		string = string[:idx - cnt] + string[idx:]
		return remove_run_rec(string, 1, 1, string[0])

	return remove_run_rec(string, idx + 1, 1, first)


def remove_run(string):
	return remove_run_rec(string, 1, 1, string[0])

print(remove_run("aabbbacd"))