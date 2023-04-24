clear all

fpath = 'Z:\data\shulan\animal training\piston_twowhisker\#23776F\New folder';
phase = '3_0';
animal = '#23776F';
previous_state_num = 10;
teach_ratio = 0.5;  % teaching signal is given when performance is worth than 0.5

files = dir(fullfile(fpath,'*.csv'));
idx_f = [];

for i = 1:length(files)
    if contains(files(i).name, sprintf('phase%s', phase))
        idx_f = [idx_f, i];
    end
end

[~, I] = sortrows(struct2table(files(idx_f)), 'name');

files_phase = files(idx_f(I));

data = [];
data_count = 1;
current_file = 0;
for i = 1:length(files_phase)
    if i <= current_file
        continue;
    end
    T = readtable(fullfile(fpath, files_phase(i).name));
    data(data_count).date = files_phase(i).name(end-22:end-13);
    data(data_count).phase = phase;
    data(data_count).dist = T.Var1;
    data(data_count).lick = T.Var2;
    data(data_count).valve = T.Var3;
    data(data_count).pistonGO = T.Var4;
    data(data_count).pistonNOGO = T.Var5;
    data(data_count).GOperformance = uint16(T.Var6);
    data(data_count).NOGOperformance = uint16(T.Var7);
    data(data_count).t = T.Var8;
    current_file = i;
    for j = i+1:length(files_phase)
        if ~strcmp(files_phase(j).name(end-22:end-13), data(data_count).date)
%             data_count = j-1;
            current_file = j-1;
            break
        end
        T = readtable(fullfile(fpath, files_phase(j).name));
        data(data_count).dist = [data(data_count).dist; data(data_count).dist(end)+T.Var1];
        data(data_count).lick = [data(data_count).lick; T.Var2];
        data(data_count).valve = [data(data_count).valve; T.Var3];
        data(data_count).pistonGO = [data(data_count).pistonGO; T.Var4];
        data(data_count).pistonNOGO = [data(data_count).pistonNOGO; T.Var5];
        data(data_count).GOperformance = [data(data_count).GOperformance; uint16(T.Var6)];
        data(data_count).NOGOperformance = [data(data_count).NOGOperformance; uint16(T.Var7)];
        data(data_count).t = [data(data_count).t; data(data_count).t(end)+T.Var8];
        current_file = j;
    end
    if ~isempty(find(diff(data(data_count).t) < 0))
        idx_timeshift = find(diff(data(data_count).t) < 0);
        for n = 1:length(idx_timeshift)
            if n < length(idx_timeshift)
                data(data_count).t(idx_timeshift(n)+1:idx_timeshift(n+1)) = data(data_count).t(idx_timeshift(n)+1:idx_timeshift(n+1)) +  data(data_count).t(idx_timeshift(n));
            else 
                data(data_count).t(idx_timeshift(n)+1:end) = data(data_count).t(idx_timeshift(n)+1:end) +  data(data_count).t(idx_timeshift(n));
            end
        end
    end
    data_count = data_count+1;
end

if phase=='1'   % lick for reward
    for i = 1:length(data)
        a = diff(data(i).t);
        lick_idx = find(data(i).lick);
        lick_dur = a(lick_idx+1);
        idx_remove = find(lick_dur>=0.5);
        lick_idx(idx_remove) = [];
        lick_dur(idx_remove) = [];
        data(i).lick_time = data(i).t(lick_idx);
        data(i).lick_rate = length(lick_idx)/(data(i).t(end)/60);  % lick/min
    end
elseif phase=='2' % GO stimuli with teaching signal
    for i = 1:length(data)
        GO_perform = data(i).GOperformance(find(diff(data(i).GOperformance)~=0)+1);
        GO_time = data(i).t(find(diff(data(i).GOperformance)~=0)+1);
        current_perform = bitget(GO_perform, 1);
        data(i).detect_rate = length(find(current_perform>0))/length(current_perform);
        data(i).detect_time = GO_time(find(current_perform>0));
        data(i).trial_time = GO_time;
        data(i).teach_time = [];
        for j = 1:length(GO_perform)
            if length(find(bitget(GO_perform(j), [10:-1:1])>0))<=previous_state_num*teach_ratio
                data(i).teach_time = [data(i).teach_time, GO_time(j)];
            end
        end
    end

elseif phase =='0' % GO stimuli without teaching signal
    for i = 1:length(data)
        GO_trial_num = length(find(diff(data(i).pistonGO)>0));
        GO_perform = data(i).GOperformance(find(diff(data(i).GOperformance)~=0)+1);
        GO_time = data(i).t(find(diff(data(i).GOperformance)~=0)+1);
        data(i).detect_rate = length(find(mod(GO_perfom,2)>0))/GO_trial_num;
        data(i).detect_time = GO_time(find(mod(GO_perfom,2)>0));
        data(i).trial_time = GO_time;
    end

elseif phase == '3_0' % GO & NOGO stimuli,with teaching signal
    for i = [1:length(data)]

        GO_perform = data(i).GOperformance(find(abs(diff(data(i).GOperformance))>1)+1);
        GO_time = data(i).t(find(abs(diff(data(i).GOperformance))>1)+1);
        current_perform = bitget(GO_perform, 1);         
        current_teach = bitget(GO_perform, 16);  % modifed 04/17/23, use on date after that only
        data(i).hit_rate = length(find(current_perform>0))/length(current_perform);
        data(i).hit_time = GO_time(find(current_perform>0));
        data(i).GO_trial_time = GO_time;
        data(i).GO_teach_time = GO_time(find(current_teach>0));

        end
        data(i).true_hit_rate = 0;   % hit without teach signal
        data(i).true_hit_time = [];
        for j = 1:length(data(i).hit_time)
            if isempty(find(data(i).GO_teach_time==data(i).hit_time(j)))
                data(i).true_hit_time = [data(i).true_hit_time, data(i).hit_time(j)];
            end
        end
        data(i).true_hit_rate = length(data(i).true_hit_time)/length(GO_time);
        
        NOGO_perform = data(i).NOGOperformance(find(abs(diff(data(i).NOGOperformance))>1)+1);
        NOGO_time = data(i).t(find(abs(diff(data(i).NOGOperformance))>1)+1);
        current_perform = bitget(NOGO_perform, 1);
        current_teach = bitget(NOGO_perform, 16);  % modifed 04/17/23, use on date after that only
        data(i).fa_rate = length(find(current_perform==1))/length(current_perform);
        data(i).fa_time = NOGO_time(find(current_perform==1));
        data(i).NOGO_trial_time = NOGO_time;
		data(i).NOGO_teach_time = NOGO_time(find(current_teach>0));

        end
        data(i).GO_teach_rate = length(data(i).GO_teach_time)/length(GO_time);
        data(i).NOGO_teach_rate = length(data(i).NOGO_teach_time)/length(NOGO_time);
        
        edges =  0:10*60:data(i).t(end);
        hit_hist = histcounts(data(i).true_hit_time, edges);
        GO_hist = histcounts(data(i).GO_trial_time, edges);
        fa_hist = histcounts(data(i).fa_time, edges);
        NOGO_hist = histcounts(data(i).NOGO_trial_time, edges);
        data(i).ROC = [fa_hist./NOGO_hist; hit_hist./GO_hist];
        data(i).ROC(isnan(data(i).ROC))=0;
        data(i).ROC = data(i).ROC';
        data(i).ROC = sortrows(data(i).ROC,1);
    end


elseif phase == '3' % GO & NOGO stimuli, reward at hit
    for i = [1:length(data)]
        GO_perform = data(i).GOperformance(find(abs(diff(data(i).GOperformance))>1)+1);
        GO_time = data(i).t(find(abs(diff(data(i).GOperformance))>1)+1);
        current_perform = bitget(GO_perform, 1);
        data(i).hit_rate = length(find(current_perform>0))/length(current_perform);
        data(i).hit_time = GO_time(find(current_perform>0));
        data(i).GO_trial_time = GO_time;

        NOGO_perform = data(i).NOGOperformance(find(abs(diff(data(i).NOGOperformance))>1)+1);
        NOGO_time = data(i).t(find(abs(diff(data(i).NOGOperformance))>1)+1);
        current_perform = bitget(NOGO_perform, 1);
        data(i).fa_rate = length(find(current_perform==1))/length(current_perform);
        data(i).fa_time = NOGO_time(find(current_perform==1));
        data(i).NOGO_trial_time = NOGO_time;

        
        edges =  0:10*60:data(i).t(end);
        hit_hist = histcounts(data(i).hit_time, edges);
        GO_hist = histcounts(data(i).GO_trial_time, edges);
        fa_hist = histcounts(data(i).fa_time, edges);
        NOGO_hist = histcounts(data(i).NOGO_trial_time, edges);
        data(i).ROC = [fa_hist./NOGO_hist; hit_hist./GO_hist];
        data(i).ROC(isnan(data(i).ROC))=0;
        data(i).ROC = data(i).ROC';
        data(i).ROC = sortrows(data(i).ROC,1);
    end
    
end


save(fullfile(fpath, sprintf('%s_phase%s.mat', animal, phase)), 'data')