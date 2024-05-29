# Arbiter
Arbiter is a lightweight unit-test library written in C. Arbiter isn't a command-line tool. Rather, it is a header file and a single C source file that you compile with your unit tests to generate an executable.

## Example Usage
To describe the usage

## Available options
Arbiter uses the following object-like preprocessor macros to modify its behaviour:
<table>
    <thead>
        <tr>
            <th>Option name</th>
            <th>Possilbe Values</th>
            <th>Description</th>
            <th>Default</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan=2><code>ARBITER_VERBOSE</code></td>
            <td rowspan=1>0</td>
            <td>Prints all status of all unit tests and summary.</td>
			<td rowspan=2>0</td>
        </tr>
        <tr>
            <td rowspan=1>1</td>
            <td rowspan=>Prints only failed unit tests and summary.</td>
        </tr>
        <tr>
          <td rowspan=1><code>ARBITER_STDERR_LOG_DIR</code></td>
            <td rowspan=1>"&ltlocation&gt"</td>
            <td>Location of folder to store the <code>stderr</code> logs. <br>
            <b>NOTE:</b> The double quotation marks are necessary.</td>
			<td rowspan=2>"tests-stderr"</td>
        </tr>
    </tbody>
</table>

These object-like macros can be set in `arbiter.h` or passed in during your compile step:

```shell
gcc -D'ARBITER_VERBOSE=1' -D'ARBITER_STDERR_LOG_DIR="tests-stderr-logs"' -Iarbiter/include/arbiter.h -o tests unit-tests.c arbiter/src/arbiter.c
```
