clear all

fpath = 'Z:\data\shulan\animal training\piston_twowhisker\#23777F\New folder'; % folder where the files are saved
phase = '1';   % 1, 2 or 3
animal = '#23777F';   % ear tag number


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

elseif phase == '2' % GO & NOGO stimuli,with teaching signal
    for i = [1:length(data)]
        data(i).GO_trial_start = find(diff(data(i).pistonGO)>0)+1;
        data(i).GO_trial_end = find(diff(data(i).pistonGO)<0)+1;
        data(i).NOGO_trial_start = find(diff(data(i).pistonNOGO)>0)+1;
        data(i).NOGO_trial_end = find(diff(data(i).pistonNOGO)<0)+1;
        data(i).hit_rate = data(i).GOperformance(find(diff(data(i).GOperformance)~=0)+1);
        data(i).hit_time = data(i).t(find(diff(data(i).GOperformance)~=0)+1);
        data(i).fa_rate = data(i).NOGOperformance(find(diff(data(i).NOGOperformance)~=0)+1);
        data(i).fa_time = data(i).t(find(diff(data(i).NOGOperformance)~=0)+1);
        
        if length(data(i).GO_trial_start)> length(data(i).GO_trial_end)
            data(i).GO_trial_end(length(data(i).GO_trial_end)+1) = length(data(i).t);
        end
        data(i).true_hit_rate = zeros(size(data(i).GO_trial_start));
        data(i).true_hit_time = zeros(size(data(i).GO_trial_start));
        count = 0;
        for j = 1:length(data(i).GO_trial_start)
            if find(data(i).lick(data(i).GO_trial_start(j):data(i).GO_trial_end(j)))>1
                count = count + 1;
                data(i).true_hit_rate(j) = count + 1;
                data(i).true_hit_time(j) = data(i).t(data(i).GO_trial_end(j));
            end
        end
        
        edges =  0:5*60:data(i).t(end);
        hit_hist = histcounts(data(i).hit_time, edges);
        GO_hist = histcounts(data(i).t(data(i).GO_trial_end), edges);
        fa_hist = histcounts(data(i).fa_time, edges);
        NOGO_hist = histcounts(data(i).t(data(i).NOGO_trial_end), edges);
        data(i).ROC = [fa_hist./NOGO_hist; hit_hist./GO_hist];
        data(i).ROC(isnan(data(i).ROC))=0;
        data(i).ROC = data(i).ROC';
        data(i).ROC = sortrows(data(i).ROC,1);
    end


elseif phase == '3' % GO & NOGO stimuli,without teaching signal
    for i = [1:length(data)]
        data(i).GO_trial_start = find(diff(data(i).pistonGO)>0)+1;
        data(i).GO_trial_end = find(diff(data(i).pistonGO)<0)+1;
        data(i).NOGO_trial_start = find(diff(data(i).pistonNOGO)>0)+1;
        data(i).NOGO_trial_end = find(diff(data(i).pistonNOGO)<0)+1;
        data(i).hit_rate = data(i).GOperformance(find(diff(data(i).GOperformance)~=0)+1);
        data(i).hit_time = data(i).t(find(diff(data(i).GOperformance)~=0)+1);
        data(i).fa_rate = data(i).NOGOperformance(find(diff(data(i).NOGOperformance)~=0)+1);
        data(i).fa_time = data(i).t(find(diff(data(i).NOGOperformance)~=0)+1);
        
        edges =  0:5*60:data(i).t(end);
        hit_hist = histcounts(data(i).hit_time, edges);
        GO_hist = histcounts(data(i).t(data(i).GO_trial_end), edges);
        fa_hist = histcounts(data(i).fa_time, edges);
        NOGO_hist = histcounts(data(i).t(data(i).NOGO_trial_end), edges);
        data(i).ROC = [fa_hist./NOGO_hist; hit_hist./GO_hist];
        data(i).ROC(isnan(data(i).ROC))=0;
        data(i).ROC = data(i).ROC';
        data(i).ROC = sortrows(data(i).ROC,1);
    end


    
end


save(fullfile(fpath, sprintf('%s_phase%s.mat', animal, phase)), 'data')