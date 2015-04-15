L1 = 68E-6; % class D filter inductor
L2 = 3.4E-6; %class E feed choke / 2
R1 = 15; % class E stage impedance / 2
C1 = 0.22E-6; % class D filter capacitor
Ts = 1/400E3; %sample time for system

H_s= tf([L2, R1], [L1*L2*C1, L1*R1*C1, L1+L2, R1]);
H_s_d = c2d(H_s, Ts);
filter = c2d(tf([1],[8E-6, 0]), Ts);
H_s_fb = feedback(H_s_d*filter*0.5, 1);