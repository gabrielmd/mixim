function read_data(type)

    format long;

    data

    output_tmp = nbDataPacketsForwarded + nbDataPacketsSent;
    input_tmp = nbDataPacketsForwarded + nbDataPacketsReceived;

    if type == 1
        plot_both(input_tmp, output_tmp);
    elseif type == 2
        plot_relation(input_tmp, output_tmp);
    elseif type == 3
        plot_consumption(devicetotalmWs);
    end

end

function plot_consumption(devicetotalmWs)

    bar(devicetotalmWs);
    legend('device total mWs');

end

function plot_both(input_tmp, output_tmp)

    output_arr = zeros(1, size(input_tmp, 2)*2);
    input_arr = zeros(1, size(input_tmp, 2)*2);

    for i=1:size(input_tmp, 2)
        output_arr(((i-1)*2)+1) = output_tmp(i);
    end

    for i=1:size(input_tmp, 2)
        input_arr(i*2) = input_tmp(i);
    end

    hold;
    bar(output_arr)
    bar(input_arr, 'r')

    legend('transmitted packets','received packets');

end

function plot_relation(input_tmp, output_tmp)
    bar(input_tmp./output_tmp);
    legend('received packets / transmitted packets');
end
