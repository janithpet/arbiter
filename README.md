# Arbiter
Arbiter is a lightweight unit-test library written in C. Arbiter isn't a command-line tool. Rather, it is a header file and a single C source file that you compile with your unit tests to generate an executable.

## Example Usage

## Usage



## Available options
Arbiter uses the following object-like preprocessor macros to modify its behaviour:
<table style="margin-left: auto; margin-right: auto;">
    <tbody>
        <tr>
            <td rowspan=2>ARBITER_VERBOSE</td>
            <td rowspan=1>0</td>
            <td>Prints all status of all unit tests and summary.</td>
        </tr>
        <tr>
            <td rowspan=1>1</td>
            <td rowspan=>Prints only failed unit tests and summary.</td>
        </tr>
        <tr>
          <td rowspan=1>ARBITER_STDERR_LOG_DIR</td>
            <td rowspan=1>string</td>
            <td>Location of folder to store the `stderr` logs.</td>
        </tr>
    </tbody>
</table>
