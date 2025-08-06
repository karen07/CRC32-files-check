#!/bin/sh

prog_name="CRC32-files-check"
exec_path="../build/$prog_name"
test_folder="test"
test_time="10"

files_count="10"
files_size="3000000"

test_file_num="5"

rm -rf "$test_folder"
pkill -f "$prog_name"

if [ ! -x "$exec_path" ]; then
    echo "$exec_path not found"
    echo "ALLFAIL"
    exit 1
fi

# Folder arg
echo "Checking that the folder is set as an argument"
run_res="$($exec_path)"
grep_run_res="$(echo "$run_res" | grep "The program need correct check folder")"
if [ -z "$grep_run_res" ]; then
    echo "ALLFAIL"
    exit 1
else
    echo "GOOD"
    echo ""
fi
# Folder arg

# Folder time arg
echo "Checking that the time is set as an argument"
run_res="$($exec_path -f "$test_folder")"
grep_run_res="$(echo "$run_res" | grep "The program need correct check folder time")"
if [ -z "$grep_run_res" ]; then
    echo "ALLFAIL"
    exit 1
else
    echo "GOOD"
    echo ""
fi
# Folder time arg

# Folder exist
echo "Checking that the folder exists"
run_res="$($exec_path -f "$test_folder" -t "$test_time")"
grep_run_res="$(echo "$run_res" | grep "Can't open folder")"
if [ -z "$grep_run_res" ]; then
    echo "ALLFAIL"
    exit 1
else
    echo "GOOD"
    echo ""
fi
# Folder exist

mkdir "$test_folder"

# Gen random files
for i in $(seq 1 "$files_count"); do
    tr -dc A-Za-z0-9 </dev/urandom | head -c "$files_size" >"$test_folder/$i.txt"
done
# Gen random files

# Check by signal
echo "Files integrity check via USR1"
"$exec_path" -f "$test_folder" -t "$test_time" >/dev/null &
pid="$(pgrep -f "$prog_name")"
sleep 1
kill -s USR1 "$pid"
sleep 1
out_res="$(journalctl | grep " $prog_name\[$pid\]" | grep "OK")"
if [ -z "$out_res" ]; then
    echo "ALLFAIL"

    kill -s TERM "$pid"
    rm -rf "$test_folder"
    exit 1
else
    echo "GOOD"
    echo ""
fi
kill -s TERM "$pid"
# Check by signal

# Check by signal ENV
echo "Files integrity check via USR1 ENV"
export CHECK_FOLDER="$test_folder"
export CHECK_FOLDER_TIME="$test_time"
"$exec_path" >/dev/null &
pid="$(pgrep -f "$prog_name")"
sleep 1
kill -s USR1 "$pid"
sleep 1
out_res="$(journalctl | grep " $prog_name\[$pid\]" | grep "OK")"
if [ -z "$out_res" ]; then
    echo "ALLFAIL"

    kill -s TERM "$pid"
    rm -rf "$test_folder"
    exit 1
else
    echo "GOOD"
    echo ""
fi
kill -s TERM "$pid"
unset CHECK_FOLDER
unset CHECK_FOLDER_TIME
# Check by signal ENV

# Check by timeout
echo "Files integrity check via Timeout"
"$exec_path" -f "$test_folder" -t "$test_time" >/dev/null &
pid="$(pgrep -f "$prog_name")"
sleep "$((test_time + 1))"
out_res="$(journalctl | grep " $prog_name\[$pid\]" | grep "OK")"
if [ -z "$out_res" ]; then
    echo "ALLFAIL"

    kill -s TERM "$pid"
    rm -rf "$test_folder"
    exit 1
else
    echo "GOOD"
    echo ""
fi
kill -s TERM "$pid"
# Check by timeout

# Check by signal file change
echo "Files not integrity check via USR1"
"$exec_path" -f "$test_folder" -t "$test_time" >/dev/null &
pid="$(pgrep -f "$prog_name")"
sleep 1
echo 1 >>"$test_folder/$test_file_num.txt"
kill -s USR1 "$pid"
sleep 1
out_res="$(journalctl | grep " $prog_name\[$pid\]" | grep "FAIL $test_folder/$test_file_num.txt")"
if [ -z "$out_res" ]; then
    echo "ALLFAIL"

    kill -s TERM "$pid"
    rm -rf "$test_folder"
    exit 1
else
    echo "GOOD"
    echo ""
fi
kill -s TERM "$pid"
# Check by signal file change

# Check by timeout file change
echo "Files not integrity check via Timeout"
"$exec_path" -f "$test_folder" -t "$test_time" >/dev/null &
pid="$(pgrep -f "$prog_name")"
sleep 1
echo 1 >>"$test_folder/$test_file_num.txt"
sleep "$((test_time + 1))"
out_res="$(journalctl | grep " $prog_name\[$pid\]" | grep "FAIL $test_folder/$test_file_num.txt")"
if [ -z "$out_res" ]; then
    echo "ALLFAIL"

    kill -s TERM "$pid"
    rm -rf "$test_folder"
    exit 1
else
    echo "GOOD"
    echo ""
fi
kill -s TERM "$pid"
# Check by timeout file change

# Check by correct timeout
echo "Checking that the signal does not break Timeout"
"$exec_path" -f "$test_folder" -t "$test_time" >/dev/null &
pid="$(pgrep -f "$prog_name")"
sleep "$((test_time + 1))"
kill -s USR1 "$pid"
sleep "$((test_time + 1))"
out_res=$(journalctl | grep " $prog_name\[$pid\]" | grep "OK")
start_date=$(echo "$out_res" | head -n 1 | cut -d' ' -f-3)
end_date=$(echo "$out_res" | tail -n 1 | cut -d' ' -f-3)
start_date_s=$(date --date "$start_date" +%s)
end_date_s=$(date --date "$end_date" +%s)
diff=$((end_date_s - start_date_s))
if [ "$diff" != "$test_time" ]; then
    echo "ALLFAIL"

    kill -s TERM "$pid"
    rm -rf "$test_folder"
    exit 1
else
    echo "GOOD"
    echo ""
fi
kill -s TERM "$pid"
# Check by correct timeout

rm -rf "$test_folder"
pkill -f "$prog_name"

echo "ALLGOOD"
exit 0
