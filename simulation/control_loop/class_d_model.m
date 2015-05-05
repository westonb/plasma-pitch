L1 = 68E-6; % class D filter inductor
L2 = 3.4E-6; %class E feed choke / 2
R1 = 15; % class E stage impedance / 2
C1 = 0.22E-6; % class D filter capacitor
Ts = 1/400E3; %sample time for system
k_p= 0.082;
k_i=65926*Ts;

H_s= tf([L2, R1], [L1*L2*C1, L1*R1*C1, L1+L2, R1]);
H_s_d = c2d(H_s, Ts);
filter= tf([k_p],[1])+tf([1],[k_i,0]);
filter_d = c2d(filter, Ts);
H_s_fb = feedback(filter_d*H_s_d, 1);
