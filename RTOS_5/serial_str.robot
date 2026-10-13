*** Settings ***
Library           SerialLibrary
Suite Setup       Connect Serial
Suite Teardown    Disconnect Serial

*** Variables ***
${COM}       COM7
${BAUD}      115200
${BOARD}     nRF5340

*** Test Cases ***
Test String Length
    ${read}=    Send Time And Read Result    000120
    Should Be Equal As Strings    ${read}    80X

    ${read}=    Send Time And Read Result    12345
    Should Be Equal As Strings    ${read}    -1X

Test Numeric Characters
    ${read}=    Send Time And Read Result    000120
    Should Be Equal As Strings    ${read}    80X

    ${read}=    Send Time And Read Result    00AB20
    Should Be Equal As Strings    ${read}    -2X

Test Seconds Boundary
    ${read}=    Send Time And Read Result    000059
    Should Be Equal As Strings    ${read}    59X

    ${read}=    Send Time And Read Result    000060
    Should Be Equal As Strings    ${read}    -3X

Test Minutes Boundary
    ${read}=    Send Time And Read Result    005959
    Should Be Equal As Strings    ${read}    3599X

    ${read}=    Send Time And Read Result    006000
    Should Be Equal As Strings    ${read}    -3X

Test Hours Boundary
    ${read}=    Send Time And Read Result    230001
    Should Be Equal As Strings    ${read}    1X

    ${read}=    Send Time And Read Result    240001
    Should Be Equal As Strings    ${read}    -3X

Test Nonzero Time
    ${read}=    Send Time And Read Result    000001
    Should Be Equal As Strings    ${read}    1X

    ${read}=    Send Time And Read Result    000000
    Should Be Equal As Strings    ${read}    -4X

*** Keywords ***
Send Time And Read Result
    [Arguments]    ${time}
    Reset Input Buffer
    Write Data    ${time}X    encoding=ascii
    ${read}=    Read Until    terminator=88    encoding=ascii
    Log To Console    Sent ${time}X, received ${read}
    RETURN    ${read}

Connect Serial
    Log To Console    Connecting to ${BOARD} on ${COM}
    Add Port    ${COM}    baudrate=${BAUD}    encoding=ascii
    Port Should Be Open    ${COM}
    Reset Input Buffer
    Reset Output Buffer

Disconnect Serial
    Log To Console    Disconnecting ${BOARD}
    Delete Port    ${COM}