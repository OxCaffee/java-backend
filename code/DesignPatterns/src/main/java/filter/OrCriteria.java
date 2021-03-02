package main.java.filter;

import java.util.ArrayList;
import java.util.List;

public class OrCriteria implements Criteria{

    private Criteria criteria;
    private Criteria otherCriteria;

    public OrCriteria(Criteria criteria, Criteria otherCriteria) {
        this.criteria = criteria;
        this.otherCriteria = otherCriteria;
    }

    @Override
    public List<Person> meetCriteria(List<Person> persons) {
        List<Person> firstCriteriaPersons = criteria.meetCriteria(persons);
        List<Person> otherCriteriaPersons = otherCriteria.meetCriteria(persons);

        List<Person> orPersons = new ArrayList<>();
        for(Person person : firstCriteriaPersons){
            if(otherCriteriaPersons.contains(person)){
                orPersons.add(person);
            }
        }
        return orPersons;
    }
}
