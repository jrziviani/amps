SET source=%1
SET dest=%2
SET what=%3
SET source=%source:/=\%
SET dest=%dest:/=\%

copy %source%\%what% %dest%